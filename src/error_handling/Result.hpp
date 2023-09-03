#pragma once

#include <expected>
#include <string_view>
#include <cstdio>

namespace venture {

class error_view : public std::string_view
{
public:
    explicit error_view(std::string_view msg) : std::string_view(msg) {}
    void log() noexcept { std::printf("%s\n", this->data()); }
};

template<typename T>
using Result = std::expected<T, error_view>;

using Error = std::unexpected<error_view>;

} // venture
