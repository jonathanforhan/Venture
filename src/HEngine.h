#ifndef HYDROGEN_SRC_HENGINE_H
#define HYDROGEN_SRC_HENGINE_H
#include "HApi.h"
#include "HRenderer.h"
#include "HWindow.h"
#include "HResult.h"

H_BEGIN

typedef struct HEngine
{
    HWindow window;
    HRenderer renderer;
} HEngine;

HResult HEngine_create(HEngine *engine);
void HEngine_destroy(HEngine *engine);

HResult HEngine_run(HEngine *engine);

H_END

#endif // HYDROGEN_SRC_HENGINE_H
