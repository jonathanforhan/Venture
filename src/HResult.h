#ifndef HYDROGEN_SRC_UTIL_HRESULT_H
#define HYDROGEN_SRC_UTIL_HRESULT_H

H_BEGIN

typedef enum HResult
{
    HResult_OK          = 0,
    HResult_ERR         = 1,
    HResult_OUT_OF_MEM  = 2,
} HResult;

H_END

#endif // HYDROGEN_SRC_UTIL_HRESULT_H
