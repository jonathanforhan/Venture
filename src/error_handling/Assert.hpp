#pragma once

#include <cstdio>
#include <cstdlib>

#ifndef V_DIST
#define vassert(Expr) do { if (!(Expr)) { fprintf(stderr, "Failed vassert: %s, File: %s, Line: %d\n", #Expr, __FILE__, __LINE__); exit(1); } } while (0)
#else
#define vassert(Expr)
#endif
