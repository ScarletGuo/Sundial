#include "log.h"
#include "manager.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

LogManager::LogManager()
{
    _buffer_size = 64 * 1024 * 1024;
    _buffer = new char[_buffer_size]; // 64 MB
    _lsn = 0;
}

LogManager::LogManager(char * log_name)
{
    assert(512 % sizeof(LogRecord) == 0);   // group commit alignment
    _buffer_size = 64 * 1024 * 1024;
    _buffer = new char[_buffer_size]; // 64 MB
    _lsn = 0;
    _name_size = 50;
    _log_name = new char[_name_size];
    strcpy(_log_name, log_name);
    //TODO: delete O_TRUNC when recovery is needed.
    _log_fd = open(log_name, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0755);
    if (_log_fd == 0) {
        perror("open log file");
        exit(1);
    }
    _log_fp = fdopen(_log_fd, "w+");
    if (_log_fp == NULL) {
        perror("open log file");
        exit(1);
    }
    
    // group commit
    flush_buffer_ = new char[_buffer_size];
    latch_ = new std::mutex();
    cv_ = new std::condition_variable();
    appendCv_ = new std::condition_variable();
}

LogManager::~LogManager() {
		delete[] _log_name;
		_log_name = nullptr;
		close(_log_fd);
}

void
LogManager::log(uint32_t size, char * record)
{
    uint32_t lsn = ATOM_ADD(_lsn, size);
    uint32_t start = lsn % _buffer_size;
    if (lsn / _buffer_size == (lsn + size) / _buffer_size) {
        memcpy(_buffer + start, record, size);
    } else {
        uint32_t tail_size = _buffer_size - start;
        memcpy(_buffer + start, record, tail_size);
        memcpy(_buffer, record + tail_size, size - tail_size);
    }
    INC_FLOAT_STATS(log_size, size);
    // TODO should write buffer to disk. For now, assume NVP or battery backed DRAM.
    if (write(_log_fd, record, size) == -1) {
			perror("write");
			exit(1);
    }
    if (fsync(_log_fd) == -1) {
        perror("fsync");
        exit(1);
    }
}

Message* LogManager::log(Message *msg) {
    Message::Type result = Message::LOG_ACK;
    if (msg->get_type() == Message::COMMIT_REQ) {
        log_message(msg, LogRecord::COMMIT);
        #if COMMIT_ALG == TWO_PC
            result = Message::COMMIT_ACK;
        #endif
    } else {
        // LogRecord::Type vote = check_log(msg);
        LogRecord::Type vote = LogRecord::INVALID;
        // invalid means no log exists
        if (vote == LogRecord::INVALID) {
            // insert watermark
            // txn_lsn_table.insert(pair<uint64_t, uint64_t>(msg->get_txn_id(), _lsn));
            if (msg->get_type() == Message::PREPARED_COMMIT) {
                // vote yes
                log_message(msg, LogRecord::YES);
                result = Message::YES_ACK;
            } else {
                // vote no
                log_message(msg, LogRecord::ABORT);
                #if COMMIT_ALG == TWO_PC
                    result = Message::ABORT_ACK;
                #endif
            }
        } else {
            if (vote != LogRecord::ABORT && msg->get_type() == Message::ABORT_REQ) {
                log_message(msg, LogRecord::ABORT);
                #if COMMIT_ALG == TWO_PC
                    result = Message::ABORT_ACK;
                #endif
            } else {
                // log exists
                result = log_to_message(vote);
            }
        }
    }
    Message * result_msg;
    #if COMMIT_ALG == ONE_PC
    if (result == Message::LOG_ACK) {
        result_msg = new Message(result, get_last_lsn());
    } else {
        result_msg = new Message(result, -1);
    }
    #else
        result_msg = new Message(result, 0);
    #endif
    return result_msg;
}

// if no log return INVALID else return the log type
LogRecord::Type LogManager::check_log(Message * msg) {
    LogRecord::Type vote = LogRecord::INVALID;
    uint16_t watermark = msg->get_lsn();
    if (watermark == -1) {
        // no log for this txn
        return vote;
    }
    FILE * fp = fopen(_log_name, "r");
    fseek(fp, 0, SEEK_END);
    fseek(fp, -sizeof(LogRecord), SEEK_CUR);
    LogRecord cur_log;
    if (fread((void *)&cur_log, sizeof(LogRecord), 1, fp) != 1) {
        return vote;
    }

    while (cur_log.get_latest_lsn() > watermark) {
        //TODO: whether to add log type equals?
        if (cur_log.get_txn_id() == msg->get_txn_id() && 
        log_to_message(cur_log.get_log_record_type()) == msg->get_type()) {
            // log exists
            vote = cur_log.get_log_record_type();
            break;
        }

        if (fseek(fp, -2 * sizeof(LogRecord), SEEK_CUR) == -1)
            break;
        assert(fread((void *)&cur_log, sizeof(LogRecord), 1, fp) == 1);
    }
    fseek(fp, 0, SEEK_END);
    fclose(fp);
    return vote;
}

void LogManager::log_message(Message *msg, LogRecord::Type type) {
    unique_lock<mutex> latch(*latch_);
    ATOM_FETCH_ADD(_lsn, 1);
    // printf("latest lsn: %d\n", _lsn);
    if (logBufferOffset_ >= _buffer_size) {
        needFlush_ = true;
        cv_->notify_one(); //let RunFlushThread wake up.
        appendCv_->wait(latch, [&] {return logBufferOffset_ < _buffer_size;});
        // logBufferOffset_ = 0;
    }
    LogRecord log{msg->get_dest_id(), msg->get_txn_id(), 
                _lsn, type};
    memcpy(_buffer + logBufferOffset_, &log, sizeof(log));
    logBufferOffset_ += sizeof(LogRecord);
    // if (fwrite(&log, sizeof(log), 1, _log_fp) != 1) {
	// 		perror("fwrite");
	// 		exit(1);
    // }
    /*
    fflush(_log_fp);
    if (fsync(_log_fd) == -1) {
        perror("fsync");
        exit(1);
    }
    */
}

uint64_t LogManager::get_last_lsn() {
    return _lsn;
}

Message::Type LogManager::log_to_message(LogRecord::Type vote) {
    switch (vote)
    {
    case LogRecord::INVALID :
        return Message:: LOG_ACK;
    case LogRecord::COMMIT :
        return Message:: LOG_COMMIT;
    case LogRecord::ABORT :
        return Message:: LOG_ABORT;
    case LogRecord::YES :
        return Message:: LOG_PREPARED_COMMIT;
    default:
        assert(false);
    }
}

//group commit
// spawn a separate thread to wake up periodically to flush
void LogManager::run_flush_thread() {
    if (ENABLE_LOGGING) return;
    ENABLE_LOGGING = true;
    flush_thread_ = new thread([&] {
        while (ENABLE_LOGGING) { //The thread is triggered every LOG_TIMEOUT seconds or when the log buffer is full
        unique_lock<mutex> latch(*latch_);
        // (2) When LOG_TIMEOUT is triggered.
        cv_->wait_for(latch, LOG_TIMEOUT, [&] {return needFlush_;});
        assert(flushBufferSize_ == 0);
        if (logBufferOffset_ > 0) {
            swap(_buffer,flush_buffer_);
            swap(logBufferOffset_,flushBufferSize_);
            // disk_manager_->WriteLog(flush_buffer_, flushBufferSize_);
            // printf("write\n");
            // TODO: figure out how to handle buffersize not reach alignment
            /*
            if (flushBufferSize_ != _buffer_size) {
                // printf("fcntl\n");
                fcntl(_log_fd, F_SETFD, O_RDWR | O_CREAT | O_TRUNC | O_APPEND);
                if (write(_log_fd, flush_buffer_, flushBufferSize_) == -1) {
                    perror("write1");
                    exit(1);
                }
            } else {
                fcntl(_log_fd, F_SETFD, O_RDWR | O_CREAT | O_TRUNC | O_APPEND | O_DIRECT);
                */
                if (write(_log_fd, flush_buffer_, flushBufferSize_) == -1) {
                    perror("write2");
                    exit(1);
                }
            // }
            if (fsync(_log_fd) == -1) {
                perror("fsync");
                exit(1);
            }
            flushBufferSize_ = 0;
            // SetPersistentLSN(lastLsn_);
        }
        needFlush_ = false;
        appendCv_->notify_all();
        }
    });
};
/*
 * Stop and join the flush thread, set ENABLE_LOGGING = false
 */
void LogManager::stop_flush_thread() {
  if (!ENABLE_LOGGING) return;
  ENABLE_LOGGING = false;
  flush(true);
  flush_thread_->join();
  assert(logBufferOffset_ == 0 && flushBufferSize_ == 0);
  delete flush_thread_;
};

void LogManager::flush(bool force) {
  unique_lock<mutex> latch(*latch_);
  if (force) {
    needFlush_ = true;
    cv_->notify_one(); //let RunFlushThread wake up.
    if (ENABLE_LOGGING)
      appendCv_->wait(latch, [&] { return !needFlush_; }); //block append thread
  } else {
    appendCv_->wait(latch);// group commit,  But instead of forcing flush,
    // you need to wait for LOG_TIMEOUT or other operations to implicitly trigger the flush operations
  }
}
