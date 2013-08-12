
class Token{
  public:
    enum TokenTag{
      #define TOK(X) X,
      #include "tokentag.def"
      TokenNum
    };

  public:
    Token(TokenTag t):
      tag_(t){}
    string toString(){}
    uint32_t getValue() const{ return 0 ;}
    float getValue() const { return 0.0; }
    bool is(tok::TokenTag){
      return tag_ == tt;
    }
    Token getKind(){
      return tag_;
    }
  private:
    TokenTag tag_;
};
class Num : public Token{
  public:
    Num(uint32_t v)//strtol()
     : Token(tok::numeric)
      ,value_(v){}
    uint32_t getValue() const{
      return value_;
    }
  private:
    uint32_t value_;//longest 4 bytes
};
class Real: public Token{
  public:
    Real(float f)
      :Token(tok::real_const)
       ,value_(f){}
    float getValue() const{
      return value_;
    }
  private:
    float value_;
};
class Word: public Token{
  public:
    Word(string s, TokenTag t):
      Token(t)
      ,lexeme_(s){}
  private:
    string lexeme_;
};
