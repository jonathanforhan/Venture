#include "Engine.hpp"
#include "error_handling/Log.hpp"

using namespace venture;

int main()
{
    try {
        Engine engine;
        engine.run();
    } catch (const std::exception &e) {
        log(Error, "Exception caught in main FATAL " << e.what());
        std::exit(EXIT_FAILURE);
    }
}
