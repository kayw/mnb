#ifndef MSTRING_H_77DE958B_3746_497A_844A_1BBE7C9018E7
#define MSTRING_H_77DE958B_3746_497A_844A_1BBE7C9018E7

namespace mnb {

class StringRef {
  public:
    //STL iterator
    typedef const char* const_iterator;
    typedef char* iterator;

    StringRef(char* begin, char* end)
      : pos_(begin), len_(end - begin) {}

    const_iterator begin() const { return pos_; }
    const_iterator end() const { return pos_ + len_; }
  private:
    char* pos_;
    int len_;
};

#ifndef _USE_NATIVE_IMPL_STRING
#include <string>
typedef std::string MString;

#else

class MString {
  public:
    static bool toInteger(const StringRef& rString, int& val);
};

}

#endif

#endif /* MSTRING_H_77DE958B_3746_497A_844A_1BBE7C9018E7 */

