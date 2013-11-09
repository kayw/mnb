#ifndef MMACROS_H_
#define MMACROS_H_ //todo uuidgen

#include <stdint.h>
#include <assert.h>
//#include <string.h>
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  TypeName operator=(const TypeName&)

//renaming sed replace all
#define SAFE_DELETE(P)  \
  if (P){               \
    delete P;           \
    P = NULL;        \
  }
namespace mnb{
    template<class T> inline const T& max_t(const T& a, const T& b) {
        return a > b ? a:b;
    }
    template<class T> inline const T& min_t(const T& a, const T& b) {
        return a < b ? a:b;
    }
}

#endif
