#ifdef 
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

namespace mnb{
    template<class T> inline const T& max_t(const T& a, const T& b) {
        return a > b ? a:b;
    }
}
