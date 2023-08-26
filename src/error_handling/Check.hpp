#pragma once
#include <cstdio>

/** check and checkf _DO_ run the expression even if building distribution */

#ifndef V_DIST
//--- throw runtime error if not true
#define check(Expr) do { if (!(Expr)) { throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
//--- throw runtime error if not true and print string
#define checkf(Expr, ...) do { if (!(Expr)) { fprintf(stderr, __VA_ARGS__); puts("\n"); throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
//--- throw runtime error, evaluated only in debug and release modes stripped from dist builds
#define check_DEBUG(Expr) check(Expr)
//--- throw runtime error, evaluated only in debug and release modes stripped from dist builds
#define checkf_DEBUG(Expr, ...) checkf(Expr, __VA_ARGS__)
#else
//--- run expression only
#define check(Expr) (void)(Expr)
//--- run expression only
#define checkf(Expr, ...) (void)(Expr)
//--- does nothing
#define check_DEBUG(Expr)
//--- does nothing
#define checkf_DEBUG(Expr, ...)
#endif
