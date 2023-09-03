#pragma once

#include "Window.hpp"

namespace venture {

class IRenderer
{
public:
    IRenderer() = delete;
	IRenderer(const IRenderer&) = delete;
	void operator=(const IRenderer&) = delete;

    IRenderer(Window* window) : _window(window) {}

    virtual void draw() = 0;

protected:
    Window *_window;
};

} // venture
