namespace mnb{
namespace expr{
#include "symbols.h"
#include "lexer.h"
class Parser{
  public:
  enum BinaryPreced{
    kUnknown      = 0,  // not binaray operator
    kComma        = 1,  // ,
    kLogicalOR    = 2,  // OR ||
    kLogicalAND   = 3,  // AND &&
    kInclusiveOR  = 4,  // |
    kInclusiveXOR = 5,  // XOR ^
    kInclusiveAND = 6,  // &
    kEquality     = 7,  // <> != ==
    kRelational   = 8,  // > >= < <= 
    kShift        = 9,  // >> <<
    kAdditive     = 10, // + -
    kMultiplicate = 11, // * / MOD %
//    kLogicalNOT   = 12, // NOT ~
//    kBuiltinCalls = 13, // sin
  };

  Parser(Lexer l)
    :lexer_(l)
    ,lookahead_(NULL){
    //lexer_ = l;
    ConsumeToken();// prime token
  }
  void ConsumeToken(){
    if(lookahead_)
      lookahead_->release();
    lookahead_ = lexer_.scan();
  }
  Token nextLookAheadToken(int aheadNum) {
    Token* tmp;
    for (int i = 0; i < aheadNum; ++i){
      tmp = lexer_.scan();
      cachedLookAheads_.push_back(tmp);
    }
    return tmp;
  }

  void matchRHSPunct(Token::TokenTag t){
    if (lookahead_.tag_ == t)
      ConsumeToken();
    else
      error("Parser expect token " << getNameByTag(t) << ",but next token is " << getNameByTag(lookahead_.tag_) );
  }
  void error(){}

  private:
  Sematic sema_;
  ErrorReporter Er_;
  Token* lookahead_;
  std::vector<Token*> cachedLookAheads_;
  IdentifierTable symbol_table_;
  //ExprResult parseExpression(){
  //  decls();
  //  Expr E = boolExpression();
  //  return E.Eval();
  //}
  //void parseDecls(){ // D->type ID
  //  while(lookahead_.tag_ == Token::kw_TYPE){
  //    Type* t = declType();
  //    Token tok = lookahead_;
  //    match(Token::identifer);
  //    match(Token::semi);
  //    ID* id = new ID((Word*)tok, t);
  //    identifier_table_[tok] = id;
  //    used_ += p.width_;
  //  }
  //}
  //Type* parseDeclType(){
  //  Type* pT = (Type*)lookahead_;
  //  match(Token::kw_TYPE);
  //  if (lookahead_.tag_ != '[') {
  //    return pT;
  //  }
  //  else{
  //    return dimType(pT);
  //  }
  //}
  //Type* parseDimType(Type* pT){
  //  match('[');
  //  Token tok = lookahead_;
  //  match(Token::numeric);
  //  match(']');
  //  if (lookahead_.tag_ == '[') {
  //    pT = parseDimType(pT);
  //  }
  //  return new Array1D((Num)tok.value_, pT);
  //}

  //ExprResult* parseBoolLiteral();
  //ExprResult* parseAssignment();
  //ExprResult* parseLogicalOR();
  //parseLogicalAnd();
  //parseNot();
  //parseEquality();
  //parseRelational();
  //parseShift();
  //parseTerm();
  //parseUnary();
  //parseFunction();
  //parseFactor();

  ExprResult parseExpression();
  ExprResult parseAssignment();
  ExprResult parseBinaryExprRHS(ExprResult lhs, BinaryPreced minPrec);
