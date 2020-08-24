#pragma once
#include "message.h"
#include "global.h"
#include "helper.h"
#include <map>
#include "log_record.h"

//group commit
#include <condition_variable>
#include <future>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>

class LogManager {
public:
    LogManager();
    ~LogManager();
    LogManager(char *log_name);
    void log(uint32_t size, char * record);
    Message* log(Message *msg);
    
    void run_flush_thread();
    void stop_flush_thread();
    void flush(bool force);

private:
    uint32_t _buffer_size;
    char * _buffer;
    uint32_t _lsn;
    FILE * _log_fp;
    int _log_fd;
    uint32_t _name_size;
    char * _log_name;
    // map<uint64_t, uint64_t> txn_lsn_table; // TODO: no garbage collection for now

    //group commit
  	char *flush_buffer_;
    uint32_t logBufferOffset_ = 0;
    uint32_t flushBufferSize_ = 0;
    bool ENABLE_LOGGING = false;
    std::chrono::duration<long long int, std::micro> log_timeout =
        std::chrono::microseconds(LOG_TIMEOUT);
    bool needFlush_ = false; //for group commit
    std::condition_variable * appendCv_; // for notifying append thread	
    // latch for cv
    std::mutex * latch_;
    // flush thread
    std::thread *flush_thread_;
    // for notifying flush thread
    std::condition_variable * cv_;

    InOutQueue * local_out_queue; // for flush thread send msg

    LogRecord::Type check_log(Message * msg);
    void log_message(Message *msg, LogRecord::Type type);
    uint64_t get_last_lsn();
    Message::Type log_to_message(LogRecord::Type vote);

    // for debugging
    uint64_t _first_log_start_time = 0;
    uint64_t max_flushing_time = 0;
    uint64_t min_flushing_time = 2000 * BILLION / 1000000;
};
