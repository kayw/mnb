#ifndef PARSER_H_BD268073_D685_494F_8AAE_64A53D2D9DCE
#define PARSER_H_BD268073_D685_494F_8AAE_64A53D2D9DCE

#include "lexer.h"
#include "symbols.h"
#include "sematics.h"

namespace mnb{
namespace expr{
class Parser{
  public:
  enum BinaryPreced{
    kUnknown      = 0,  // not binaray operator
    kComma        ,  // ,
    kAssign       ,  // =
    kLogicalOR    ,  // OR ||
    kLogicalAND   ,  // AND &&
    kInclusiveOR  ,  // |
    kInclusiveXOR ,  // XOR ^
    kInclusiveAND ,  // &
    kEquality     ,  // <> != ==
    kRelational   ,  // > >= < <= 
    kShift        ,  // >> <<
    kAdditive     , // + -
    kMultiplicate , // * / MOD %
//    kLogicalNOT   = 12, // NOT ~
//    kBuiltinCalls = 13, // sin
  };

  explicit Parser(ErrorReport& reporter)
    :Er_(reporter), lookahead_(NULL), pLexer_(NULL) {} 

  ExprResult ParseExpression(const MString& exprStr);


  private:
    Sematic sema_;
    ErrorReport& Er_;
    Token* lookahead_;
    std::vector<Token*> cachedLookAheads_;
    IdentifierTable symbol_table_;
    Lexer* pLexer_;// as a pointer refrence the scope lexer

    bool skipUntil(Token::TokenTag t) {
      while(!lookahead_->is(t) && !lookahead_->is(Token::unknown)){
        ConsumeToken();
      }
      return lookahead_->is(t);
    }

    void ConsumeToken() {
      if(lookahead_)
        lookahead_->release();
      lookahead_ = pLexer_->scan();
    }

    Token* nextLookAheadToken(int aheadNum) {
      Token* tmp;
      for (int i = 0; i < aheadNum; ++i){
        tmp = pLexer_->scan();
        cachedLookAheads_.push_back(tmp);
      }
      return tmp;
    }

    void matchRHSPunct(Token::TokenTag t) {
      if (lookahead_->is(t) )
        ConsumeToken();
      else
        //error("Parser expect token " << getNameByTag(t) << ",but next token is " << getNameByTag(lookahead_.tag_) );
        Diag(diag::err_expected_token) << t;
    }

    ErrorBuilder Diag(int32_t errid){
      return pLexer_->Diag(errid);
    }

    bool parseDeclarator();
    void parseBracketDeclarator(Declarator& D);
    ExprResult parsePrimaryExpression();
    ExprResult parseInitializer();
    ExprResult parseBraceInitialier();
    bool isDeclarationSpecifier();
    QualType parseDeclSpecifier();
    ExprResult parseAssignment();
    ExprResult parseParenExpression();
    ExprResult parsePostfixSuffix(ExprResult Lhs);
    bool parseExprListFailed(ExprVector& exprs);
    ExprResult parseBinaryExprRHS(ExprResult lhs, BinaryPreced minPrec);
    static BinaryPreced getBinOpPrecedence(const Token::TokenTag kind);
};

}
}

#endif /* PARSER_H_BD268073_D685_494F_8AAE_64A53D2D9DCE */

