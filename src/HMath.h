#ifndef HYDROGEN_SRC_HMATH_H
#define HYDROGEN_SRC_HMATH_H
#include "HApi.h"

H_BEGIN

#define H_MIN(_A, _B) ({                                                        \
    __typeof__ (_A) _Min_A = (_A);                                              \
    __typeof__ (_B) _Min_B = (_B);                                              \
    _Min_A < _Min_B ? _Min_A : _Min_B;                                          \
})

#define H_MAX(_A, _B) ({                                                        \
    __typeof__ (_A) _Max_A = (_A);                                              \
    __typeof__ (_B) _Max_B = (_B);                                              \
    _Max_A < _Max_B ? _Max_A : _Max_B;                                          \
})

#define H_CLAMP(_X, _Max, _Min) (H_MIN(_Max, H_MAX(_X, _Min)))

H_END

#endif // HYDROGEN_SRC_HMATH_H
