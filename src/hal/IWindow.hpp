#pragma once

namespace venture {

class IWindow
{
public:
    [[nodiscard]] virtual bool should_close() noexcept = 0;
    virtual void poll_events() noexcept = 0;
};

} // venture
