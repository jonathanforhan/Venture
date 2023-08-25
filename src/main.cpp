#include "Engine.hpp"
#include <iostream>

using namespace venture;

int main()
{
    try {
        Engine engine;
        engine.run();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught in main FATAL " << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
