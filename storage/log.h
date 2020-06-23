#pragma once
#include "message.h"
#include "global.h"
#include "helper.h"
#include <map>
#include "log_record.h"

class LogManager {
public:
    LogManager();
    ~LogManager();
    LogManager(char *log_name);
    void log(uint32_t size, char * record);
    Message* log(Message *msg);
private:
    uint32_t _buffer_size;
    char * _buffer;
    uint32_t _lsn;
    FILE * _log_fp;
    int _log_fd;
    uint32_t _name_size;
    char * _log_name;
    // map<uint64_t, uint64_t> txn_lsn_table; // TODO: no garbage collection for now

    LogRecord::Type check_log(Message * msg);
    void log_message(Message *msg, LogRecord::Type type);
    uint64_t get_last_lsn();
    Message::Type log_to_message(LogRecord::Type vote);
};