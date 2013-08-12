namespace mnb{
namespace expr{
ExprResult Parser::ParseExpression(){
  if (isDeclarationSpecifier() ) {
    parseDeclarator();
  }
  else{
    ExprResult lhs(parseAssignment() );
    return parseBinaryExprRHS(lhs, kAssign);
  }
}
void Parser::parseDeclarator() {
  QualType T = parseDeclSpecifier();
  if (lookahead_.is(Token::identifer) ){
    IdentifierInfo* pII = lookahead_.getIdentifierInfo();
    Declarator D(dt, pII ); 
    //IdentifierInfo& ii = lookahead_.getIdentifierInfo();
    //ii.setType(dt);
    ConsumeToken();
    while(true){
      if (lookahead_.is(Token::lsquare) )
        parseBracketDeclarator(D, T);
      else
        break;
    }
    VarDecl* decl = sema_.analyzeDeclarator(D);
    assert(decl);
    if (lookahead_.is(Token::equal) ){
      ConsumeToken();
      ExprResult initialier(parseInitializer() );
      if (initialier.isInvalid() ) {
        skipUntil(Token::comma);
      }
      decl->setInitialier(initialier);
    }
    else if (lookahead_.is(Token::semi){
      ConsumeToken();
      //decl->setInitialier(0);
      sema_.actOnUninitializedDecl(decl);
    }
    else{
      errorReport.diagnose(getColumn(), "no semi or equal sign for declarator") << decl->getName();
      //diag assert(false && ");
      
    }
  }
  else{
    // diag
      errorReport.diagnose(getColumn(), "not identifer found after decl specifier") << T->getName();
  }
}

void Parser::parseBracketDeclarator(Declarator& D, const QualType T){
  ConsumeBracket();

  if (lookahead_.is(Token::r_square) {
    matchRHSPunct(Token::r_square);
    D.addTypeInfo(D.createArrayChunk(NULL);
  }
  else if (lookahead_.is(Token::numeric)
          && nextLookAheadToken(1).is(Token::r_square) ) {
    ExprNode* idxNode = new ExprNode(T, lookahead_.value() );
    ConsumeToken();
    matchRHSPunct(Token::r_square);
    D.addTypeInfo(D.createArrayChunk(idxNode) );
  }
  else{
    errorReport.diagnose(getColumn(), "Syntax error after ") << D.;
    skipUntil(Token::r_square);
  }
}

ExprResult Parser::parseInitializer(){
  if (lookahead_.isNot(Token::l_brace) ) {
    return parseAssignment();
  }
  else{
    return parseBraceInitialier();
  }
}

ExprResult Parser::parseBraceInitialier(){
  ExprNode* subElt;
  bool initExprOK = true;
  std::vector<ExprNode*> InitExprVec;
  while(1){
    subElt = parseInitializer().get();
    if (!subElt.isInvalid() ) {
      initExprOK = false;
      if (lookahead_.isNot(Token::comma)) {
        skipUntil(Token::r_brace);
        break;
      }
    }
    else if (subElt.getExprClass() == kInitExprsClass) {
      diag
        break;
    }
    else{
      InitExprVec.push_back(subElt);
    }
    if (lookahead_.isNot(Token::comma) ) break;
    ConsumeToken();// comma lex
    // trailing comma
    if (lookahead_.is(Token::r_brace) ) break;
  }
  if (initExprOK && lookahead_.is(Token::r_brace) ) {
    return sema_.analyzeInitExprs(InitExprVec);
  }
  matchRHSPunct(Token::r_brace);
  return ExprError();
}

bool Parser::isDeclarationSpecifier(){
  return symbol_table_.findType(lookahead_.getKind() );
}
QualType Parser::parseDeclSpecifier(){
  ConsumeToken();
  Token::TokenTag tag = lookahead_.getKind();
  //assert(symbol_table_.findType(tag) );
  return QualType(symbol_table_.typeOf(tag) );
}

ExprResult parseAssignment(){
  ExprResult res;
  switch(lookahead_.getKind() ){
    case Token::l_paren:
      res = parseParenExpression();
      break;
    case Token::numeric: case Token::real_const:
    case Token::kw_TRUE: case Token::kw_ON:
    case Token::kw_FALSE: case Token::kw_OFF:
      res = sema_.analyzeConstantLiteral(lookahead_);
      ConsumeToken();
      return res;
    case Token::identifer:
      IdentifierInfo* pii = lookahead_.getIdentifierInfo();
      ConsumeToken();
      res = sema_.analyzeIdentifier(pii);//builtincallid
      break;
    case Token::tilde: case Token::minus:
      Token opTok = lookahead_;
      ConsumeToken();
      res = parseAssignment();
      if (!res.isInvalid() )
        res = sema_.analyzeUnaryOperator(opTok,res);
      return res.move(); 
    default:
      //if (lookahead_.isBuiltinCall() ){
      //  res = analyzeBuiltinCallID(lookahead_);//new BuiltinIdentifier(lookahead_.getIdentifierInfo() );
      //  break;
      //}
      return ExprError();
  }
  return parsePostfixSuffix(res);
}

ExprResult parseParenExpression(){
  consumeParen();
  ExprResult res = parseAssignment();
  if (!res.isInvalid() && lookahead_.is(Token::r_paren) ) {
    res = new ParenExpr(res); //todo
  }
  if (res.isInvalid() ){
    skipUntil(Token::r_paren);
    return ExprError();
  }
  matchRHSPunct(Token::r_paren);
  return res.move();
}

ExprResult parsePostfixSuffix(ExprResult Lhs){
  while(1){
    switch(lookahead_.getKind() ){
      case Token::l_square:
        ConsumeBracket();
        ExprResult idx = parseAssignment();
        if (!Lhs.isInvalid() && !idx.isInvalid() && lookahead_.is(Token::r_square) ) {
          Lhs = sema_.analyzeArraySubscript(Lhs, idx);
        }
        else{
          Lhs = ExprError();
        }
        matchRHSPunct(Token::r_square);
        break;
      case Token::l_paren:
        consumeParen();
        ExprVector args;
        if (!Lhs.isInvalid() && parseExprListFailed(args) ) 
          Lhs = ExprError();
          skipUntil(Token::r_paren);
        }
        else if(lookahead_.isNot(Token::r_paren) ){
          matchRHSPunct(Token::r_paren);
          Lhs = ExprError();
        }
        else{
          Lhs = sema_.analyzeBuiltinCallExpr(Lhs, args);
          consumeParen();
        }
        break;
      default:
        return Lhs.move();
    }
  }
}

bool Parser::parseExprListFailed(ExprVector& exprs, std::vector<Loc>& commaLocs){
  while(1){
   ExprResult expr = parseAssignment();
   if (expr.isInvalid() ) 
     return true;
   exprs.push_back(expr.take() );
   if (lookahead_.isNot(Token::comma) )
     return false;
   commaLocs.push_back(ConsumeToken() );
  }
}

ExprResult Parser::parseBinaryExprRHS(ExprResult lhs, BinaryPreced minPrec){
  BinaryPreced nextPrec = getBinOpPrecedence(lookahead_.getKind() );
  while (nextPrec >= minPrec){
    Token opTok = lookahead_;
    ConsumeToken();
    ExprResult rhs = parseAssignment();
    BinaryPreced thisPrec = nextPrec;
    nextPrec = getBinOpPrecedence(lookahead_.getKind() );
    bool isRightAssoc = (nextPrec == kAssign);
    if (nextPrec > thisPrec || (nextPrec == thisPrec && isRightAssoc) ){
      rhs = parseBinaryExprRHS(rhs, static_cast<BinaryPreced>(thisPrec + !isRightAssoc) );
      if (rhs.isInvalid() )
        lhs = ExprError();
      nextPrec = getBinOpPrecedence(lookahead_.getKind() );
    }
    assert((NextTokPrec <= ThisPrec) && "Recursion didn't work!");
    if (!lhs.isInvalid() ) {
      lhs = sema_.analyzeBinaryOperator(lhs, rhs, opTok);
    }
  }
  return lhs.move();
}

static BinaryPreced Parser::getBinOpPrecedence(const TokenKind kind){
  switch(kind){
    case Token::comma: 
      return kComma;
    case Token::equal:
      return kAssign;
    case Token::pipepipe:
      return kLogicalOR;
    case Token::ampamp:
      return kLogicalAND:
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
