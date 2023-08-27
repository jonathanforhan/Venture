#pragma once
#include <iostream>
#include <type_traits>

#ifdef V_LOGGING_ENABLED
namespace venture::detail::logging {
enum class Info;
enum class Warning;
enum class Error;
#ifdef V_LOG_INFO
constexpr static bool LOG_INFO = true;
#else
constexpr static bool LOG_INFO = false;
#endif
#ifdef V_LOG_WARNINGS
constexpr static bool LOG_WARNINGS = true;
#else
constexpr static bool LOG_WARNINGS = false;
#endif
#ifdef V_LOG_ERRORS
constexpr static bool LOG_ERRORS = true;
#else
constexpr static bool LOG_ERRORS = false;
#endif
} // venture::detail::logging

namespace venture {

#define log(Level, Msg) do {                                                                                           \
using namespace detail::logging;                                                                                       \
if constexpr (LOG_INFO && std::is_same<Level, Info>::value)                                                            \
{                                                                                                                      \
    std::cout << "Venture Info: " << Msg << std::endl;                                                                 \
}                                                                                                                      \
else if constexpr (LOG_WARNINGS && std::is_same<Level, Warning>::value)                                                \
{                                                                                                                      \
    std::cout << "Venture Warning: " << Msg << std::endl;                                                              \
}                                                                                                                      \
else if constexpr (LOG_ERRORS && std::is_same<Level, Error>::value)                                                    \
{                                                                                                                      \
    std::cerr << "Venture Error: " << Msg << std::endl;                                                                \
}                                                                                                                      \
} while(0)

#define logf(Level, ...) do {                                                                                          \
using namespace detail::logging;                                                                                       \
if constexpr (LOG_INFO && std::is_same<Level, Info>::value)                                                            \
{                                                                                                                      \
    fprintf(stdout, "Venture Info: ");                                                                                 \
    fprintf(stdout, __VA_ARGS__);                                                                                      \
}                                                                                                                      \
else if constexpr (LOG_WARNINGS && std::is_same<Level, Warning>::value)                                                \
{                                                                                                                      \
    fprintf(stdout, "Venture Warning: ");                                                                              \
    fprintf(stdout, __VA_ARGS__);                                                                                      \
}                                                                                                                      \
else if constexpr (LOG_ERRORS && std::is_same<Level, Error>::value)                                                    \
{                                                                                                                      \
    fprintf(stderr, "Venture Error: ");                                                                                \
    fprintf(stderr, __VA_ARGS__);                                                                                      \
}                                                                                                                      \
} while(0)
#else // V_LOGGING_ENABLED
#define log(Level, Msg)
#define logf(Level, ...)
#endif

} // venture