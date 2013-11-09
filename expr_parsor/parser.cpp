#include "parser.h"
namespace mnb{
namespace expr{

ExprResult Parser::ParseExpression(const std::string& exprStr){
  Lexer l(exprStr.c_str(),symbol_table_, Er_);
  pLexer_ =  &l;
  sema_.setSemaDependency(pLexer_, symbol_table_);
  ConsumeToken();// prime token
  if (isDeclarationSpecifier() ) {
    parseDeclarator();
  } else {
    ExprResult lhs(parseAssignment() );
    return parseBinaryExprRHS(lhs, kAssign);
  }
  return ExprResult(true);
}

void Parser::parseDeclarator() {
  QualType T = parseDeclSpecifier();
  ConsumeToken();
  if (lookahead_->is(Token::identifer) ){
    const MString& decl_name = lookahead_->toString();
    const IdentifierInfo* pII = lookahead_->getIdentifierInfo();
    Declarator D(T, pII ); 
    //IdentifierInfo& ii = lookahead_.getIdentifierInfo();
    //ii.setType(dt);
    ConsumeToken();
    while(true){
      if (lookahead_->is(Token::l_square) )
        parseBracketDeclarator(D);
      else
        break;
    }
    VarDecl* decl = sema_.analyzeDeclarator(D);
    assert(decl);
    if (lookahead_->is(Token::equal) ){
      ConsumeToken();
      ExprResult initialier = parseInitializer();
      //ExprResult initialier(parseInitializer() );
      if (initialier.isInvalid() ) {
        Diag(diag::err_expected_expression);
        skipUntil(Token::comma);
        initialier = ExprError();
      }
      decl->setInitialier(initialier);//todo analyzeassignment
    }
    else if (lookahead_->is(Token::semi)){
      ConsumeToken();
      sema_.actOnUninitializedDecl(decl);
    }
    else{
      //errorReport.diagnose(getColumn(), "no semi or equal sign for declarator") << decl->getName();
      //diag assert(false && ");
      Diag(diag::err_expected_semi_or_equal_declaration) << decl_name;
    }
  }
  else{
    // diag
      //errorReport.diagnose(getColumn(), "not identifer found after decl specifier") << T->getName();
      Diag(diag::err_expect_identifer) << T.get()->getTypeLiteral();
  }
}

void Parser::parseBracketDeclarator(Declarator& D) {
  matchRHSPunct(Token::l_square);
  if (lookahead_->is(Token::r_square)) {
    matchRHSPunct(Token::r_square);
    D.addTypeInfo(D.createArrayChunk(NULL) );
  }
  else if (lookahead_->is(Token::numeric)
          && nextLookAheadToken(1)->is(Token::r_square) ) {
    Constant* idxNode = new Constant(lookahead_->getValue(), QualType(symbol_table_.typeOf(Token::kw_INT) ) );
    ConsumeToken();
    //matchRHSPunct(Token::r_square);FIXME: cachedLookAheads_.rbegin match
    D.addTypeInfo(D.createArrayChunk(idxNode) );
  }
  else{
    skipUntil(Token::r_square);
    //errorReport.diagnose(getColumn(), "Syntax error after ") << D.;
  }
}

ExprResult Parser::parseInitializer(){
  if (!lookahead_->is(Token::l_brace) ) {
    return parseAssignment();// lhs(parseAssignment); return parseBinaryExprRHS();  todo  a = 2+3
  } else{
    matchRHSPunct(Token::l_brace);
    return parseBraceInitialier();
  }
}

ExprResult Parser::parseBraceInitialier(){
  ExprNode* subElt;
  bool initExprOK = true;
  std::vector<ExprNode*> InitExprVec;
  while(1){
    ExprResult subResult = parseInitializer();
    subElt = subResult.get();
    if (subResult.isInvalid() ) {
      initExprOK = false;
      if (!lookahead_->is(Token::comma)) {
        skipUntil(Token::r_brace);
        break;
      }
    } else if (subElt->getExprClass() == ExprNode::kInitExprsClass) { 
      Diag(diag::err_braceinitialiazer_is_initexpr);
      break;
    } else {
      InitExprVec.push_back(subElt);
    }
    if (!lookahead_->is(Token::comma) ) break;
    ConsumeToken();// comma lex
    // trailing comma
    if (lookahead_->is(Token::r_brace) ) break;
  }
  if (initExprOK && lookahead_->is(Token::r_brace) ) {
    matchRHSPunct(Token::r_brace);
    return sema_.analyzeInitExprs(InitExprVec);
  }
  skipUntil(Token::r_brace);
  return ExprError();
}

bool Parser::isDeclarationSpecifier(){
  return symbol_table_.findType(lookahead_->getKind() );
}

QualType Parser::parseDeclSpecifier(){
  Token::TokenTag tag = lookahead_->getKind();
  //assert(symbol_table_.findType(tag) );
  return QualType(symbol_table_.typeOf(tag) );
}

ExprResult Parser::parseAssignment() {
  ExprResult res(true);
  switch(lookahead_->getKind() ){
    case Token::l_paren:
      res = parseParenExpression();
      break;
    case Token::numeric: case Token::real_const:
    case Token::kw_TRUE: case Token::kw_ON:
    case Token::kw_FALSE: case Token::kw_OFF:
      {
        res = sema_.analyzeConstantLiteral(lookahead_);
        ConsumeToken();
        return res;
      }
    case Token::identifer: 
      {
        const IdentifierInfo* pii = lookahead_->getIdentifierInfo();
        ConsumeToken();
        res = sema_.analyzeIdentifier(pii);//builtincallid
      }
      break;
    case Token::tilde: case Token::minus:
      {
        Token::TokenTag opTokKind = lookahead_->getKind();
        ConsumeToken();
        res = parseAssignment();
        if (!res.isInvalid() )
          res = sema_.analyzeUnaryOperator(opTokKind, res.get());
        return res;//res.move(); 
      }
    default:
      //if (lookahead_.isBuiltinCall() ){
      //  res = analyzeBuiltinCallID(lookahead_);//new BuiltinIdentifier(lookahead_.getIdentifierInfo() );
      //  break;
      //}
      Diag(diag::err_expected_expression);
      return ExprError();
  }
  return parsePostfixSuffix(res);
}

ExprResult Parser::parseParenExpression(){
  //consumeParen();
  matchRHSPunct(Token::l_paren);
  ExprResult res = parseAssignment();
  ExprNode* pParenNode = NULL;
  if (!res.isInvalid() && lookahead_->is(Token::r_paren) ) {
    pParenNode = new ParenExpr(res); //todo
  }
  if (res.isInvalid() ){
    skipUntil(Token::r_paren);
    return ExprError();
  }
  matchRHSPunct(Token::r_paren);
  return ExprResult(pParenNode);//res.move();
}

ExprResult Parser::parsePostfixSuffix(ExprResult Lhs){
  while(1){
    switch(lookahead_->getKind() ){
      case Token::l_square: {
          matchRHSPunct(Token::l_square);
          ExprResult idx = parseAssignment();
          if (!Lhs.isInvalid() && !idx.isInvalid() && lookahead_->is(Token::r_square) ) {
            Lhs = sema_.analyzeArraySubscript(Lhs, idx);
          } else{
            Lhs = ExprError();
          }
          matchRHSPunct(Token::r_square);
        }
        break;
      case Token::l_paren: {
          matchRHSPunct(Token::l_paren);
          ExprVector args;
          if (!Lhs.isInvalid() && parseExprListFailed(args) ) {
            Lhs = ExprError();
            skipUntil(Token::r_paren);
          } else if(!lookahead_->is(Token::r_paren) ) {
            matchRHSPunct(Token::r_paren);
            Lhs = ExprError();
          } else {
            Lhs = sema_.analyzeBuiltinCallExpr(Lhs, args);
            matchRHSPunct(Token::r_paren);
          }
        }
        break;
      default:
        return Lhs;//Lhs.move();
    }
  }
}

bool Parser::parseExprListFailed(ExprVector& exprs) {
  while(1){
   ExprResult expr = parseAssignment();
   if (expr.isInvalid() ) 
     return true;
   exprs.push_back(expr);
   if (!lookahead_->is(Token::comma) )
     return false;
   ConsumeToken();
  }
}

ExprResult Parser::parseBinaryExprRHS(ExprResult lhs, BinaryPreced minPrec){
  BinaryPreced nextPrec = getBinOpPrecedence(lookahead_->getKind() );
  while (nextPrec >= minPrec){
    Token::TokenTag opTokKind = lookahead_->getKind();
    ConsumeToken();
    ExprResult rhs = parseAssignment();
    BinaryPreced thisPrec = nextPrec;
    nextPrec = getBinOpPrecedence(lookahead_->getKind() );
    bool isRightAssoc = (nextPrec == kAssign);
    if (nextPrec > thisPrec || (nextPrec == thisPrec && isRightAssoc) ){
      rhs = parseBinaryExprRHS(rhs, static_cast<BinaryPreced>(thisPrec + !isRightAssoc) );
      if (rhs.isInvalid() )
        lhs = ExprError();
      nextPrec = getBinOpPrecedence(lookahead_->getKind() );
    }
    assert((nextPrec <= thisPrec) && "Recursion didn't work!");
    if (!lhs.isInvalid() && !rhs.isInvalid() ) {
      lhs = sema_.analyzeBinaryOperator(opTokKind, lhs, rhs);
    }
  }
  return lhs;//lhs.move();
}

Parser::BinaryPreced Parser::getBinOpPrecedence(const Token::TokenTag kind){
  switch(kind){
    case Token::comma: 
      return kComma;
    case Token::equal:
      return kAssign;
    case Token::pipepipe:
      return kLogicalOR;
    case Token::ampamp:
      return kLogicalAND;
    case Token::pipe:
        return kInclusiveOR;
    case Token::caret:
        return kInclusiveXOR;
    case Token::amp:
        return kInclusiveAND;
    case Token::lessgreater: case Token::exclaimequal:
    case Token::equalequal:
        return kEquality;
    case Token::greater: case Token::greaterequal:
    case Token::less:case Token::lessequal:
        return kRelational;
    case Token::lessless: case Token::greatergreater:
        return kShift;
    case Token::plus: case Token::minus:
        return kAdditive;
    case Token::star: case Token::slash:
    case Token::percent:
        return kMultiplicate;
    default:
        return kUnknown;
  }
}
}
}
