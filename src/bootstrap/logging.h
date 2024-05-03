#ifndef BITCOIN_LOGGING_H
#define BITCOIN_LOGGING_H

#include "tinyformat.h"
#include <cerrno>  // For perror
#include <cstring>  // For strerror
#include <iostream>
#include <string>

/** Get format string from VA_ARGS for error reporting */
template<typename... Args> std::string FormatStringFromLogArgs(const char *fmt, const Args&... args) { return fmt; }

#define LogPrintf(...) do {                                                         \
    if(true) {                                                       \
        std::string _log_msg_; /* Unlikely name to avoid shadowing variables */     \
        try {                                                                       \
            _log_msg_ = tfm::format(__VA_ARGS__);                                   \
        } catch (tinyformat::format_error &e) {                                     \
            /* Original format string will have newline so don't add one here */    \
            _log_msg_ = "Error \"" + std::string(e.what()) +                        \
                        "\" while formatting log message: " +                       \
                        FormatStringFromLogArgs(__VA_ARGS__);                       \
        }                                                                           \
        std::cout << _log_msg_;                                                     \
    }                                                                               \
} while(0)

#define LogPrint(category, ...) do {                                                \
    if (LogAcceptCategory((category))) {                                            \
        LogPrintf(__VA_ARGS__);                                                     \
    }                                                                               \
} while(0)

#endif