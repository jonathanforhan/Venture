#pragma once

namespace venture {

class IRenderer
{
public:
    [[nodiscard]] virtual bool should_close() = 0;
    virtual void poll_events() = 0;
    virtual void render() = 0;
};

} // venture
