//
// Created by rjcunningham on 1/8/18.
//

#ifndef SOFTWARE_LOGGER_HPP
#define SOFTWARE_LOGGER_HPP

#include <string>
#include <fstream>
#include "timestamps.hpp"

typedef std::string string;
//TODO better way to do this?

enum LOG_LEVEL {
    LOG_DEBUG_VERBOSE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,

};
//TODO current implementation holds noticeable performance penalty if logging is on.
//One easy simple fix is to use a macro or header implementation that will get inlined and check against some
//defined value such as #define LOG_ON true
//and log_head_wrap(if not LOG_ON: return, else blah);
//not really worth right now, but worth being aware of.
class Logger {
    public:
        string logfile;
        string name;
        LOG_LEVEL level;

        /**
         * Creates a logger object with a specified name and output logfile.
         * @param logfile A relative path (from the program's working directory) to create logs.
         * @param name The name of the logger to print.
         * @param level The loglevel to use for this logger.
         */
        Logger(string const &logfile, string const &name, LOG_LEVEL level);

        /**
         * Logs a message to the logfile.
         * @param message the logfile.
         * @param level The loglevel to which to log.
         */
        void log_message(const string &message, LOG_LEVEL level);

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void debugv(const string &message) {log_message(message, LOG_DEBUG_VERBOSE);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void debugv(const string &message, timestamp_t t) {log_message(message, LOG_DEBUG_VERBOSE, t);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void debug(const string &message) {log_message(message, LOG_DEBUG);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void debug(const string &message, timestamp_t t) {log_message(message, LOG_DEBUG, t);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void info(const string &message) {log_message(message, LOG_INFO);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void info(const string &message, timestamp_t t) {log_message(message, LOG_INFO, t);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void warn(const string &message) {log_message(message, LOG_WARNING);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void warn(const string &message, timestamp_t t) {log_message(message, LOG_WARNING, t);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void error(const string &message) {log_message(message, LOG_ERROR);};

        /**
         * Logs a message to the logfile verbosely.
         * @param message the logfile.
         */
        void error(const string &message, timestamp_t t) {log_message(message, LOG_ERROR, t);};

        /**
         * Logs a message to the logfile.
         * @param message the logfile.
         * @param level The loglevel to which to log.
         * @param t The time to use for the message.
         */
        void log_message(const string &message, LOG_LEVEL level,timestamp_t t);

    private:
        std::ofstream of;

};


#endif //SOFTWARE_LOGGER_HPP
