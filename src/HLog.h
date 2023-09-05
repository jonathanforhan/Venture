#ifndef HYDROGEN_SRC_UTIL_LOG_H
#define HYDROGEN_SRC_UTIL_LOG_H
#include "Api.h"
#include <stdio.h>

H_BEGIN

#ifdef H_LOG_VERBOSE
#define HLogVerbose 0x01
#else
#define HLogVerbose 0x00
#endif
#ifdef H_LOG_INFO
#define HLogInfo 0x02
#else
#define HLogInfo 0x00
#endif
#ifdef H_LOG_WARNINGS
#define HLogWarning 0x04
#else
#define HLogWarning 0x00
#endif
#ifdef H_LOG_ERRORS
#define HLogError 0x08
#else
#define HLogError 0x00
#endif

#ifdef H_LOGGING_ENABLED
#   define H_LOG_FORMATTER(_Fd, _Desc, _Fmt, ...)                               \
        fprintf(_Fd, _Desc _Fmt "%s", __VA_ARGS__)

#   define H_LOG(_Level, ...)                                                   \
do {                                                                            \
    if (_Level & HLogVerbose)                                                   \
        H_LOG_FORMATTER(stdout, "Hydrogen Verbose: ", __VA_ARGS__, "\n");       \
    else if (_Level & HLogInfo)                                                 \
        H_LOG_FORMATTER(stdout, "Hydrogen Info: ", __VA_ARGS__, "\n");          \
    else if (_Level & HLogWarning)                                              \
        H_LOG_FORMATTER(stdout, "Hydrogen Warning: ", __VA_ARGS__, "\n");       \
    else if (_Level & HLogError)                                                \
        H_LOG_FORMATTER(stderr, "Hydrogen Error: ", __VA_ARGS__, "\n");         \
} while(0)
#else
#   define H_LOG(_Level, ...) while(0)
#endif

H_END

#endif // HYDROGEN_SRC_UTIL_LOG_H
