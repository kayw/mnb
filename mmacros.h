#ifndef MMACROS_H_315FB683_DB8E_4EDE_A75A_8272DB3000DB
#define MMACROS_H_315FB683_DB8E_4EDE_A75A_8272DB3000DB

#include <stdint.h>
#include <assert.h>
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  TypeName operator=(const TypeName&)

//renaming sed replace all
#define SAFE_DELETE(P)  \
  if (P) {              \
    delete P;           \
    P = NULL;           \
  }

//Avoid "unused parameter" warnings
//qglobal.h
#if defined(Q_CC_INTEL) && !defined(Q_OS_WIN) || defined(Q_CC_RVCT)
template <typename T>
inline void qUnused(T &x) { (void)x; }
#  define M_UNUSED(x) qUnused(x);
#else
#  define M_UNUSED(x) (void)x;
#endif

namespace mnb{
    template<class T> inline const T& max_t(const T& a, const T& b) {
        return a > b ? a:b;
    }
    template<class T> inline const T& min_t(const T& a, const T& b) {
        return a < b ? a:b;
    }
}


#endif /* MMACROS_H_315FB683_DB8E_4EDE_A75A_8272DB3000DB */
