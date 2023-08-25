#pragma once
#include <cstdio>

#ifdef V_DEBUG
//--- throw runtime error if not true
#define check(Expr) do { if (!(Expr)) { throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
#elif defined V_RELEASE
//--- throw runtime error if not true
#define check(Expr) do { if (!(Expr)) { throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
#elif defined V_DIST
#define check(Expr) (void)Expr
#endif

#ifdef V_DEBUG
//--- throw runtime error if not true
#define checkf(Expr, ...) do { if (!(Expr)) { fprintf(stderr, __VA_ARGS__); throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
#elif defined V_RELEASE
//--- throw runtime error if not true
#define checkf(Expr, ...) do { if (!(Expr)) { fprintf(stderr, __VA_ARGS__); throw std::runtime_error("Fatal error '" #Expr "' has failed runtime check " __FILE__); } } while (0)
#elif defined V_DIST
#define checkf(Expr) (void)Expr
#endif
