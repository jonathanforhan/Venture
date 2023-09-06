#ifndef HYDROGEN_SRC_HWINDOW_H
#define HYDROGEN_SRC_HWINDOW_H
#include <stdbool.h>
#include <stdint.h>
#include "HApi.h"
#include "HResult.h"

H_BEGIN

typedef enum HWindowOpts
{
    HWindowOptsNull        = 0,
    HWindowOptsResizable   = 1,
    HWindowOptsCentered    = 2,
} HWindowOpts;

typedef struct HWindow
{
    void *impl;
} HWindow;

void HWindow_show(HWindow *window);
bool HWindow_should_close(HWindow *window);
void HWindow_poll_events();

H_END

#endif // HYDROGEN_SRC_HWINDOW_H
