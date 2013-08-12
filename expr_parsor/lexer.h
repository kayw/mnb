namespace mnb{
namespace expr{
#include "token.h"
class Lexer{
  public:
    Lexer(const char* buffers)
      :bufferPtr_(buffers)
       ,bufferBegin_(buffers)
       ,tokenColumn_(0){}
    void scan(Token& tok);
    char advanceChar(const char* curPtr){
      //if(curPtr[0] == L'x'){ wchar_t
      //  ++curPtr;
      //  return '*'
      //}
      //++tokenColumn_;//all curptr++ into advanceChar;//todo
      return *curPtr++;
    }
    char getNextChar(const char* curPtr){
      return curPtr[0];
    }
    const char* consumeNextChar(const char* curPtr){
      return curPtr + 1;
    }
    bool nextCharIs(char ch, const char* curPtr){
      if (curPtr[0] == ch) {
        curPtr = consumeNextChar(curPtr);
        return true;
      }
      return false;
    }
  private:
    typedef signed short exponent_t;
    char peek_;
    const char* bufferPtr_;
    int tokenColumn_;
    IdentifierTable& identifier_table_;
};
}
}
