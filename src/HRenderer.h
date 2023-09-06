#ifndef HYDROGEN_SRC_HRENDERER_H
#define HYDROGEN_SRC_HRENDERER_H
#include "HApi.h"
#include "HWindow.h"
#include "HResult.h"

H_BEGIN

typedef struct HRenderer
{
    void *impl;
} HRenderer;

H_END

#endif // HYDROGEN_SRC_HRENDERER_H
