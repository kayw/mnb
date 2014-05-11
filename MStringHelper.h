#include <stdint.h>
namespace mnb {

// Utility to convert a character to a digit in a given base
template<typename CHAR, uint8_t BASE, bool BASE_LTE_10> class BaseCharToDigit {};

// Faster specialization for bases <= 10
template<typename CHAR, uint8_t BASE> class BaseCharToDigit<CHAR, BASE, true> {
 public:
  static bool Convert(CHAR c, uint8_t* digit) {
    bool converted = false;
    if (c >= '0' && c < '0' + BASE) {
      *digit = c - '0';
      converted = true;
    }
    return converted;
  }
};

// Specialization for bases where 10 < base <= 36
template<typename CHAR, uint8_t BASE> class BaseCharToDigit<CHAR, BASE, false> {
 public:
  static bool Convert(CHAR c, uint8_t* digit) {
    bool converted = true;
    if (c >= '0' && c <= '9') {
      *digit = c - '0';
    } else if (c >= 'a' && c < 'a' + BASE - 10) {
      *digit = c - 'a' + 10;
    } else if (c >= 'A' && c < 'A' + BASE - 10) {
      *digit = c - 'A' + 10;
    } else {
      converted = false;
    }
    return converted;
  }
};

template<uint8_t BASE, typename CHAR> bool CharToDigit(CHAR c, uint8_t* digit) {
  return BaseCharToDigit<CHAR, BASE, BASE <= 10>::Convert(c, digit);
}

}
