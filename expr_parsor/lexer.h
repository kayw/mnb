#ifndef LEXER_H_CAC68713_FF3B_4C6C_B540_1C4B7CF575CA
#define LEXER_H_CAC68713_FF3B_4C6C_B540_1C4B7CF575CA

#include "mmacros.h"
#include "token.h"
#include "errors.h"

namespace mnb{
namespace expr{

class IdentifierTable;

class Lexer{
  public:
    Lexer(const char* buffers, IdentifierTable& table, ErrorReport& reporter)
      : bufferPtr_(buffers) ,bufferBegin_(buffers)
       ,tokenColumn_(0) , identifier_table_(table)
       ,reporter_(reporter){}
    Token* scan();
    char advanceChar(const char*& curPtr){
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
    bool nextCharIs(char ch, const char*& curPtr){
      if (curPtr[0] == ch) {
        curPtr = consumeNextChar(curPtr);
        return true;
      }
      return false;
    }
    ErrorBuilder Diag(const int32_t diagid){
      return reporter_.diagnose(diagid, tokenColumn_);
    }

  private:
    typedef signed short exponent_t;
    const char* bufferPtr_;
    const char* bufferBegin_;
    int tokenColumn_;
    IdentifierTable& identifier_table_;
    ErrorReport& reporter_;

    Token* lexNumericToken(const char*& curPtr);
    void readNumeric(const char*& curPtr, char& peek, uint32_t* inital);
    exponent_t readExponent(const char*& curPtr);
    Token* lexIdentifier(const char*& curPtr);
};

}
}

#endif /* LEXER_H_CAC68713_FF3B_4C6C_B540_1C4B7CF575CA */

