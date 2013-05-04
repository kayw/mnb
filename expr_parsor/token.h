
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
    Num(int32_t v)//strtol()
     : Token(tok::numeric)
      ,value_(v){}
  private:
    int32_t value_;//longest 4 bytes
};
class Real: public Token{
  public:
    Real(float f)
      :Token(tok::real_const)
       ,value_(f){}
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
