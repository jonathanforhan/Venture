#pragma once

namespace venture {

class IWindow
{
public:
    IWindow() = default;
    IWindow(const IWindow&) = delete;
    void operator=(const IWindow&) = delete;

    [[nodiscard]]
    virtual bool should_close() noexcept = 0;
    virtual void poll_events() noexcept = 0;
};

} // venture
