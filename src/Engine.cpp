#include "Engine.hpp"

namespace venture {

Engine::Engine()
        : _renderer(Renderer())
{
}

void Engine::run()
{
    while (!_renderer.should_close())
    {
        _renderer.poll_events();
    }
}

} // venture