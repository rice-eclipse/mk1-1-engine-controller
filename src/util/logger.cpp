//
// Created by rjcunningham on 1/8/18.
//

#include <iostream>
#include "logger.hpp"

Logger::Logger(string const &logfile, string const &name, LOG_LEVEL level)
        : logfile (logfile)
        , name (name)
        , level (level)
{
    // TODO should be be opening this here?
    //of.open(logfile);
}

void Logger::log_message(string const &message, LOG_LEVEL level) {
    log_message(message, level, get_time());
}

void Logger::log_message(string const &message, LOG_LEVEL level,timestamp_t t) {
    if (level < Logger::level) {
        return;
    }
    string s = std::to_string(t);
    string l;

    switch (level) {
        case (LOG_DEBUG_VERBOSE):
            l = "DEBUGV: ";
            break;
        case (LOG_DEBUG):
            l = "DEBUG: ";
            break;
        case (LOG_INFO):
            l = "INFO: ";
            break;
        case (LOG_WARNING):
            l = "WARNING: ";
            break;
        case (LOG_ERROR):
            l = "ERROR: ";
            break;
    }

    // Open the file to append.
    of.open(logfile, of.app);

    // Log to the output file.
    of << l << name;
    of << " [" << s << "] ";
    of << message << std::endl;
    of.close();

    // Log to stdout. TODO should have some magic that does same formatting? Macro would work?
    std::cout << l << name << " [" << s << "] " << message << std::endl;
}

