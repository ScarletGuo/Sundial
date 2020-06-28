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
    _buffer_size = 64 * 1024 * 1024;
    _buffer = new char[_buffer_size]; // 64 MB
    _lsn = 0;
    _name_size = 50;
    _log_name = new char[_name_size];
    strcpy(_log_name, log_name);
    //TODO: delete O_TRUNC when recovery is needed.
    _log_fd = open(log_name, O_RDWR | O_CREAT | O_TRUNC | O_DIRECT | O_APPEND, 0755);
    if (_log_fd == 0) {
        perror("open log file");
        exit(1);
    }
    _log_fp = fdopen(_log_fd, "w+");
    if (_log_fp == NULL) {
        perror("open log file");
        exit(1);
    }
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
    } else {
        LogRecord::Type vote = check_log(msg);
        // invalid means no log exists
        if (vote == LogRecord::INVALID) {
            // insert watermark
            // txn_lsn_table.insert(pair<uint64_t, uint64_t>(msg->get_txn_id(), _lsn));
            if (msg->get_type() == Message::PREPARED_COMMIT) {
                // vote yes
                log_message(msg, LogRecord::YES);
            } else {
                // vote no
                log_message(msg, LogRecord::ABORT);
            }
        } else {
            if (vote != LogRecord::ABORT && msg->get_type() == Message::ABORT_REQ) {
                log_message(msg, LogRecord::ABORT);
            } else {
                // log exists
                result = log_to_message(vote);
            }
        }
    }
    Message * result_msg;
    if (result == Message::LOG_ACK) {
        result_msg = new Message(result, get_last_lsn());
    } else {
        result_msg = new Message(result, -1);
    }
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
    // fclose(fp);
    return vote;
}

void LogManager::log_message(Message *msg, LogRecord::Type type) {
    ATOM_FETCH_ADD(_lsn, 1);
    // printf("latest lsn: %d\n", _lsn);
    LogRecord log{msg->get_dest_id(), msg->get_txn_id(), 
                _lsn, type};
    memcpy(_buffer, &log, sizeof(log));
    // if (fwrite(&log, sizeof(log), 1, _log_fp) != 1) {
	// 		perror("fwrite");
	// 		exit(1);
    // }
    if (write(_log_fd, _buffer, sizeof(log)) == -1) {
			perror("write");
			exit(1);
    }
    // fflush(_log_fp);
    if (fsync(_log_fd) == -1) {
        perror("fsync");
        exit(1);
    }
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