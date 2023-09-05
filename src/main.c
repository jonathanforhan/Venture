#include "HEngine.h"

int main()
{
    HResult result;
    HEngine engine;

    if ((result = HEngine_create(&engine)) != HResult_OK)
        return result;

    if ((result = HEngine_run(&engine)) != HResult_OK)
        return result;

    HEngine_destroy(&engine);

    return result;
}