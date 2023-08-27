#pragma once

#include "hal/Renderer.hpp"
#include "hal/Window.hpp"

namespace venture {

class Engine final
{
public:
    explicit Engine();
    void run();

private:

private:
    Window _window;
    Renderer _renderer;
};

} // venture

