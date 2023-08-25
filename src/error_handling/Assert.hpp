#pragma once

#include <cstdio>

#ifdef V_DEBUG
#define vassert(Expr) do { if (!(Expr)) { fprintf(stderr, "Failed vassert: %s, File: %s, Line: %d\n", #Expr, __FILE__, __LINE__); } } while (0)
#else
#define vassert(Expr)
#endif
