#pragma once

#include "hal/Renderer.hpp"

namespace venture {

class Engine final
{
public:
    explicit Engine();
    void run();

private:

private:
    Renderer _renderer;
};

} // venture

