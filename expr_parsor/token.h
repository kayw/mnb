#ifndef TOKEN_H_329E9463_0E1A_47B2_9611_A2085732EDA9
#define TOKEN_H_329E9463_0E1A_47B2_9611_A2085732EDA9

#include "mmacros.h"
#include "mstring.h"

namespace mnb{
namespace expr{

class IdentifierInfo;
class Token{
  public:
    enum TokenTag{
#define TOK(X) X,
#include "tokentag.def"
      TokenNum
#undef TOK
    };

  public:
    Token(TokenTag t)
      : tag_(t){}
    virtual ~Token(){}
    virtual MString toString(){ return ""; }
    virtual uint32_t getValue() const{ return 0 ;}
    virtual float getFloatValue() const { return 0.0; }
    virtual const IdentifierInfo* getIdentifierInfo() const { return NULL;}
    void release() { delete this; }
    bool is(TokenTag tt) const { return tag_ == tt; }
    TokenTag getKind() const { return tag_; }
  private:
    TokenTag tag_;
};
class Num : public Token{
  public:
    Num(uint32_t v)//strtol()
     : Token(numeric)
      ,value_(v){}
    virtual ~Num(){}
    virtual uint32_t getValue() const{
      return value_;
    }
  private:
    uint32_t value_;//longest 4 bytes
};

class Real: public Token{
  public:
    Real(float f)
      : Token(real_const)
       ,value_(f){}
    virtual ~Real(){}
    virtual float getFloatValue() const{
      return value_;
    }
  private:
    float value_;
};

class IdentifierInfo{
  public:
    IdentifierInfo()
      :tag_(Token::identifer) {}
    Token::TokenTag tag_;
  private:
    DISALLOW_COPY_AND_ASSIGN(IdentifierInfo);    
};

class Word: public Token{
  public:
    Word(MString s, const IdentifierInfo* pII)
      :Token(pII->tag_)
       ,lexeme_(s), pIdenInfo_(pII) {}
    virtual ~Word() {}
    virtual const IdentifierInfo* getIdentifierInfo() const { return pIdenInfo_;}
    virtual MString toString(){ return lexeme_; }
  private:
    MString lexeme_;
    const IdentifierInfo* pIdenInfo_;
};

}
}

#endif /* TOKEN_H_329E9463_0E1A_47B2_9611_A2085732EDA9 */

