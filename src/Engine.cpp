#include "Engine.hpp"

namespace venture {

Engine::Engine()
        : _window(800, 600),
          _renderer(&_window)
{
    bool static exists = false;
    if (!exists) exists = true; else throw std::runtime_error("Multiple instances of Engine");
}

void Engine::run()
{
    while (!_window.should_close())
    {
        _window.poll_events();
        _renderer.draw();
    }
}

} // venture