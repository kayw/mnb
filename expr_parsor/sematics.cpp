#include "ast.h"
//using namespace mnb::expr;
namespace mnb{
namespace expr{
ExprResult Owned(ExprNode* E){
  return E;
}

ExprResult ExprError(){
  return ExprResult(false);
}

VarDecl* Sematic::analyzeDeclarator(const Declarator& D){
  QualType t = getTypeInfoBy(D);
  VarDecl* decl = new VarDecl(t);
  declGroupMap_.insert(D.getIdentifier(), decl);
  return decl;
}

QualType Sematic::getTypeInfoBy(const Declarator& D){
  QualType T = D.getQualType();
  int e = D.getTypeObjects();
  int i = 0;
  for (; e != i ; ++i) {
    DeclaratorChunk& dc = D.getChunckObject(e - i - 1);
    if (dc.getKind() == DeclaratorChunk::Array) {
      DeclaratorChunk::ArrayTypeInfo ati = dc.Arr_;
      T = buildArrayType(T, ati.Arr_.ElemSize); 
    }
  }
  return T;
}

QualType Sematic::buildArrayType(QualType T, ExprNode* arraySize){
  if (!arraySize) {
    Diag(diag::err_invalid_array_element_size);
    return T;
  }
  uint64_t constVal = arraySize.evaluate();
  ConstArrayType* pNewCAT = new ConstArrayType(T, constVal);
  return QualType(pNewCAT);
}

ExprResult Sematic::analyzeInitExprs(const std::vector<ExprNode*>& initList){
  return ExprResult(new InitExprs(initList) );
}

ExprResult Sematic::analyzeIdentifier(const IdentifierInfo* pII){  
  if(!pII)
    return ExprResult(false);
  BuiltinInfo* pbi = symbol_table_.findBuiltinIDInfo(pII->tag_);
  if(pbi) {
    FunctionDecl* pfd = FunctionDecl::createBuiltinDecl(pbi);
    return ExprResult(new BuiltinCallExpr(pfd) );
  }
  else if(declGroupMap_.find(pII) != declGroupMap_.end() ) {
    return ExprResult(new VarDecl(pII) );
  }
  else{
    return ExprResult(false);
  }
}

ExprResult Sematic::analyzeConstantLiteral(Token* pToken){
  switch(pToken->getKind() ){
    case Token::numeric:
      //Num* pNumTok = static_cast<Num*>(pToken);
      return ExprResult(new Constant(pToken->getValue()) );
    case Token::real_const:
      //Real* pRealTok = static_cast<Real*>(pToken);
      return ExprResult(new Constant(pToken->getValue()) );
    case Token::kw_TRUE: case Token::kw_ON:
      return ExprResult(new Constant(true) );
    case Token::kw_FALSE: case Token::kw_OFF:
      return ExprResult(new Constant(false) );
    default:
      return ExprResult(false);
  }
}

void Sematic::actOnUninitializedDecl(VarDecl* decl){
  QualType T = decl.getQualType();
  decl->setInitialier(T.createDefaultInit() );
}

ExprResult Sematic::analyzeBinaryOperator(const Token opTok, ExprResult lhs, ExprResult rhs){
  OperatorKind opk = getBinaryOpcode(opTok);
  QualType resultTy;
  switch(opk){
    case kBOAssign:
      resultTy = checkAssignmentOperands(lhs.get(), rhs);
      break;
    case kBOMul: 
      resultTy = checkMultiplyDivOperands(lhs, rhs, false);
      break;
    case kBODiv:
      resultTy = checkMultiplyDivOperands(lhs, rhs, true);
      break;
    case kBORem: 
      resultTy = checkRemainderOperands(lhs, rhs);
      break;
    case kBOAdd: case kBOSub:
      resultTy = checkAdditionSubtractOperands(lhs, rhs);
      break;
    case kBOShl: case kBOShr:
      resultTy = checkShiftOperands(lhs, rhs);
      break;
    case kBO_LE: case kBO_LT:
    case kBO_GE: case kBO_GT:
      resultTy = checkCompareOperands(lhs, rhs, Opc, true);
      break;
    case kBO_EQ: case kBO_NE:
      resultTy = checkCompareOperands(lhs, rhs, Opc, false);
      break;
    case kBOBitAnd: case kBOBitXor: case kBOBitOr:
      resultTy = checkBitwiseOperands(lhs, rhs);
      break;
    case kBOLAnd: case kBOLOr:
      ResultTy = checkLogicalOperands(lhs, rhs, Opc);
      break;
  }

  if (ResultTy.isNull() || lhs.isInvalid() || rhs.isInvalid())
    return ExprError();

  // Check for array bounds violations for both sides of the BinaryOperator
  CheckArrayAccess(lhs.get());
  CheckArrayAccess(rhs.get());

  return Owned(new BinaryOperator(lhs.take(), rhs.take(), Opc,
                                              ResultTy, VK, OK, OpLoc));
}

QualType Sematic::checkMultiplyDivOperands(
  ExprResult &lex, ExprResult &rex, SourceLocation Loc, bool isDiv) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  if (!lex.get()->getType()->isArithmeticType() ||
      !rex.get()->getType()->isArithmeticType())
    return invalidOperands(Loc, lex, rex);

  // Check for division by zero.
  if (isDiv && rex.get()->isConstantZero() )
    Diag(diag::warn_division_by_zero);

  return compType;
}

QualType Sematic::checkRemainderOperands(ExprResult& lex, ExprResult& rex){
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  if (!lex.get()->getType()->isIntegerType() ||
      !rex.get()->getType()->isIntegerType())
    return invalidOperands(Loc, lex, rex);

  // Check for remainder by zero.
  if (rex.get()->isConstantZero() )
    Diag(diag::warn_remainder_by_zero);

  return compType;
}

QualType Sematic::checkAdditionSubtractOperands( // C99 6.5.6
  ExprResult &lex, ExprResult &rex, SourceLocation Loc) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  if (!lex.get()->getType()->isArithmeticType() ||
      !rex.get()->getType()->isArithmeticType())
    return invalidOperands(Loc, lex, rex);

  return compType;
}

QualType Sematic::checkShiftOperands(ExprResult &lex, ExprResult &rex,
                                  SourceLocation Loc, unsigned Opc) {
  // C99 6.5.7p2: Each of the operands shall have integer type.
  if (!lex.get()->getType()->isIntegerType() || 
      !rex.get()->getType()->isIntegerType())
    return invalidOperands(Loc, lex, rex);

  //C99 6.5.7p3

  if (lex.isInvalid())
    return QualType();
  QualType LHSTy = lex.get()->getType();

  // The RHS is simpler.
  if (rex.isInvalid())
    return QualType();

  // Sanity-check shift operands
  diagnoseBadShiftValues(*this, lex, rex, Loc, Opc, LHSTy);

  // "The type of the result is that of the promoted left operand."
  return LHSTy;
}

QualType Sematic::checkCompareOperands(ExprResult &lex, ExprResult &rex,
                                    SourceLocation Loc, unsigned OpaqueOpc,
                                    bool isRelational) {
  OperatorKind Opc = (OperatorKind) OpaqueOpc;

  QualType lType = lex.get()->getType();
  QualType rType = rex.get()->getType();

    // For non-floating point types, check for self-comparisons of the form
    // x == x, x != x, x < x, etc.  These always evaluate to a constant, and
    // often indicate logic errors in the program.
    //
    // NOTE: Don't warn about comparison expressions resulting from macro
    // expansion. Also don't warn about comparisons which are only self
    // comparisons within a template specialization. The warnings should catch
    // obvious cases in the definition of the template anyways. The idea is to
    // warn when the typed comparison operator will always evaluate to the same
    // result.

  // C99 6.5.8p3 / C99 6.5.9p4
  if (lex.get()->getType()->isArithmeticType()
      && rex.get()->getType()->isArithmeticType()
      && isRelational) {
    arithmeticConversion(lex, rex);
    if (lex.isInvalid() || rex.isInvalid())
      return QualType();
  }else if (!isRelational /* == !=*/
      && lex.get()->getType()->hasIntegerRepresentation()
      && rex.get()->getType()->hasIntegerRepresentation()){
    //lex = unaryConversions(lex.take());
    //if (lex.isInvalid())
    //  return QualType();

    //rex = unaryConversions(rex.take());
    //if (rex.isInvalid())
    //  return QualType();
    arithIntegerCast(lex, rex);
  }

  // The result of comparisons is 'bool' in C++, 'int' in C.
  QualType ResultTy = BoolTy;
  return ResultTy;
}

QualType Sematic::checkBitwiseOperands(ExprResult &lex, ExprResult &rex, SourceLocation Loc) {
  ExprResult lexResult = Owned(lex), rexResult = Owned(rex);
  QualType compType = arithmeticConversion(lexResult, rexResult);
  if (lexResult.isInvalid() || rexResult.isInvalid())
    return QualType();
  lex = lexResult.take();
  rex = rexResult.take();
  if (lex.get().getType().isIntegerType() && 
      rex.get().getType().isIntegerType() )
    return compType;

  return invalidOperands(Loc, lex, rex);
}

QualType Sematic::checkLogicalOperands( // C99 6.5.[13,14]
  ExprResult &lex, ExprResult &rex, SourceLocation Loc, unsigned Opc) {
  
  // Diagnose cases where the user write a logical and/or but probably meant a
  // bitwise one.  We do this when the LHS is a non-bool integer and the RHS
  // is a constant.
  if (!lex.get()->getType()->isBooleanType() ||
      !rex.get()->getType()->isBooleanType())
    return invalidOperands(Loc, lex, rex);

  return BoolTy;

  // The following is safe because we only use this method for
  // non-overloadable operands.

  // C++ [expr.log.and]p1
  // C++ [expr.log.or]p1
  // The operands are both contextually converted to type bool.
}

QualType Sematic::checkAssignmentOperands(ExprNode* LHS, ExprResult &RHS,
                                       SourceLocation Loc) {
  // Verify that LHS is a modifiable lvalue, and emit error if not.

  QualType LHSType = LHS->getType();
  QualType RHSType =RHS.get()->getType();

  // Get canonical types.  We're not formatting these types, just comparing
  // them.
  // Common case: no conversion required.
  if (LHSType == RHSType) {
    return LHSType;
  }
  CastKind Kind =  kCastNoOp;
  switch(LHSType.getKind() ){
    case kBoolTy:
      if (RHSType.isIntegerType() )
        Kind = kCastIntegralToBoolean;
      if (RHSType.isFloatingType() )
        Kind = kCastFloatingToBoolean;
      break;
    case kIntTy:
      if (RHSType.isBooleanType() || RHSType.isIntegerType() )
        Kind = kCastIntegral;
      if (RHSType.isFloatingType() )
        Kind = kCastFloatingToIntegral;
      break;
    case kFloatTy:
      if (RHSType.isIntegerType() || RHSType.isBooleanType() )
        Kind = kCastIntegralToFloating;
      break;
  }
  // C99 6.5.16.1p2: The value of the right operand is converted to the
  // type of the assignment expression.
  // CheckAssignmentConstraints allows the left-hand side to be a reference,
  // so that we can use references in built-in functions even in C.
  // The getNonReferenceType() call makes sure that the resulting expression
  // does not have reference type.
  RHS = createCastExprByType(RHS.take(), LHSType, Kind);
  // Simple assignment "x = y".
  if (RHS.isInvalid())
    return QualType();
  // C99 6.5.16p3: The type of an assignment expression is the type of the
  // left operand unless the left operand has qualified type, in which case
  // it is the unqualified version of the type of the left operand.
  // C99 6.5.16.1p2: In simple assignment, the value of the right operand
  // is converted to the type of the assignment expression (above).
  // C++ 5.17p1: the type of the assignment expression is that of its left
  // operand.
  return LHSType; 
}

int Sematic::getIntegerTypeRank(const QualType& lhsTy, const QualType& rhsTy){
  int lhsIntWidth = lhsTy.getTypeWidth();
  int rhsIntWidth = rhsTy.getTypeWidth();
  bool lhsSigned = lhsTy.isSignedInteger();
  bool rhsSigned = rhsTy.isSignedInteger();
  if (lhsSigned == rhsSigned) {
    if (lhsIntWidth > rhsIntWidth)
      return 1;
    else if (lhsIntWidth == rhsIntWidth)
      return 0;
    else
      return -1;
  }
  if (lhsSigned) {
    if (lhsIntWidth >= rhsIntWidth)
      return 1;
    else
      return -1;
  }
  if (lhsIntWidth > rhsIntWidth)
    return 1;
  return -1;
}

QualType Sematic::arithmeticConversion(ExprResult& lhsExpr, ExprResult& rhsExpr){
  QualType lhsTy = lhsExpr.get()->getQualType();
  QualType rhsTy = rhsExpr.get()->getQualType();
  if (lhsTy == rhsTy)
    return lhsTy;
  if (lhsTy.isIntegerType() && rhsTy.isIntegerType() )
    return arithIntegerCast(lhsExpr, rhsExpr);
  bool lhsFloat = lhsTy.isFloatingType();
  bool rhsFloat = rhsTy.isFloatingType();
  // If we have an integer operand, the result is the real floating type.
  if (LHSFloat) {
    if (rhsTy.isIntegerType()) {
      // Convert rhs to the lhs floating point type.
      rhsExpr = createCastExprByType(rhsExpr.take(), lhs, kCastIntegralToFloating);
      return lhs;
    }
  }
  if (RHSFloat) {
    if (lhsTy.isIntegerType()) {
      // Convert lhs to the rhs floating point type.
      lhsExpr = createCastExprByType(lhsExpr.take(), rhs, kCastIntegralToFloating);
      return rhs;
    }
  }
}

QualType Sematic::invalidOperands(SourceLocation Loc, ExprResult &lex,
                               ExprResult &rex) {
  Diag(diag::err_typecheck_invalid_operands)
    << lex.get()->getType() << rex.get()->getType();
  return QualType();
}

void Sematic::diagnoseBadShiftValues(ExprResult &lex, ExprResult &rex,
                                   SourceLocation Loc, unsigned Opc,
                                   QualType LHSTy) {
  //llvm::APSInt Right;
  QualType RHSTy = rex.get()->getQualType();
  // Check right/shifter operand
  if (!RHSTy->isIntegerType() )
    return;

  ExprValue eval = rex.get()->evaluate();
  if (eval.isNegative() ) {
    Diag(diag::warn_shift_negative);
    return;
  }
  //llvm::APInt LeftBits(Right.getBitWidth(),
  //                     S.Context.getTypeSize(lex.get()->getType()));
  int32_t leftbits = lex.get()->getTypeWidth();
  if (eval.intVal.uintValue > leftbits){//Right.uge(LeftBits)) {
    Diag(diag::warn_shift_gt_typewidth);
    return;
  }
  if (Opc != kBOShl)
    return;

  // When left shifting an ICE which is signed, we can check for overflow which
  // according to C++ has undefined behavior ([expr.shift] 5.8/2). Unsigned
  // integers have defined behavior modulo one more than the maximum value
  // representable in the result type, so never warn for those.
  QualType LHSTy = lex.get()->getQualType();
  if (!LHSTy.isIntegerType() || !LHSTy->isSignedInteger())
    return;
  ExprValue leftvalue = lex.get()->evaluate();
  int32_t resultBits = eval.intVal.uintValue + leftvalue.getValidBits();
  if (leftbits < resultBits) //LeftBits.uge(ResultBits))
    Diag(diag::warn_shift_result_gt_typewidth) << resultBits << LHSTy << leftbits;
  //if (leftbits == resultBits - 1) {
  //  S.Diag(Loc, diag::warn_shift_result_sets_sign_bit)
  //      << LHSTy
  //      << lex.get()->getSourceRange() << rex.get()->getSourceRange();
  //  return;
  //}
  //S.Diag(Loc, diag::warn_shift_result_gt_typewidth)
  //  << HexResult.str() << Result.getMinSignedBits() << LHSTy
  //  << Left.getBitWidth() << lex.get()->getSourceRange()
  //  << rex.get()->getSourceRange();
}

void Sematic::CheckArrayAccess(const ExprNode* expr) {
  assert(expr->getStmtClass() == kArraySubscriptExprClasss );
  const ExprNode* BaseExpr = expr->getBase();
  const ExprNode* IndexExpr = expr->getIdx();

  const ConstantArrayType *ArrayTy =
    static_cast<ConstantArrayType>(BaseExpr->getQualType().get() );
  if (!ArrayTy)
    return;

  QualType indexType = IndexExpr->getQualType();
  if (!indexType.isIntegerType() )
    return;
  ExprValue index = IndexExpr.evaluate();

  if (!index.isNegative() ) {
    int32_t size = ArrayTy->getArraySize();
    if (size <= 0)
      return;
    // For array subscripting the index must be less than size, but for pointer
    // arithmetic also allow the index (offset) to be equal to size since
    // computing the next address after the end of the array is legal and
    // commonly done e.g. in C++ iterators and range-based for loops.
    if (index.intVal.uintValue >= size)
      return;
    Diag(diag::warn_array_index_exceeds_bounds)<< index.toString(10, true)
                          << size.toString(10, true)
                          << (unsigned)size.getLimitedValue(~0U);
  } else {
    Diag(diag::warn_array_index_precedes_bounds)<< index.toString(10, true);
  }

}

ExprResult
Sematic::analyzeArraySubscript(ExprNode *Base, SourceLocation LLoc,
                              ExprNode *Idx, SourceLocation RLoc) {
  ExprNode* LHSExp = Base;
  ExprNode* RHSExp = Idx;

  QualType LHSTy = LHSExp->getType(), RHSTy = RHSExp->getType();

  // C99 6.5.2.1p2: the expression e1[e2] is by definition precisely equivalent
  // to the expression *((e1)+(e2)). This means the array "Base" may actually be
  // in the subscript position. As a result, we need to derive the array base
  // and index from the expression types.
  QualType ResultType;
  if (LHSTy->isArrayElement()) {
    // C99 6.5.2.1p1
    if(!IndexExpr->getType()->isIntegerType() ) {
      Diag(diag::err_typecheck_subscript_not_integer);
      return ExprError();
    }
    ResultType = LHSExp->getType();
  } else {
    Diag(diag::err_typecheck_subscript_value);
    return ExprError();
  }
  return Owned(new ArraySubscriptExpr(LHSExp, RHSExp, ResultType, RLoc));
}

ExprResult Sematic::analyzeUnaryOperator(SourceLocation OpLoc, tok::TokenKind Kind, ExprNode* InputExpr) {
  UnaryOperatorKind Opc =  getUnaryOpcode(Kind);
  ExprResult Input = Owned(InputExpr);
  QualType resultType;
  switch (Opc) {
  case kUOPlus:
  case kUOMinus:
    //Input = unaryConversions(Input.take());
    if (Input.isInvalid()) return ExprError();
    resultType = Input.get()->getType();
    if (resultType->isArithmeticType() ) 
      break;

    Diag(diag::err_typecheck_unary_expr) << resultType;
    return ExprError();
    //return ExprError(Diag(OpLoc, diag::err_typecheck_unary_expr)
    //  << resultType << Input.get()->getSourceRange());

  case kUOLNot: // logical negation
    // Unlike +/-/~, integer promotions aren't done here (C99 6.5.3.3p5).
    //Input = DefaultFunctionArrayLvalueConversion(Input.take());
    //if (Input.isInvalid()) return ExprError();
    resultType = Input.get()->getType();
    if (!resultType->isBooleanType() ) {
      // C99 6.5.3.3p1: ok, fallthrough;
      Diag(diag::err_typecheck_unary_expr) << resultType;
      return ExprError();
    }
    
    // LNot always has type int. C99 6.5.3.3p5.
    // In C++, it's bool. C++ 5.3.1p8
    //resultType = Context.getLogicalOperationType();
    break;

  if (resultType.isNull() || Input.isInvalid())
    return ExprError();

  // Check for array bounds violations in the operand of the UnaryOperator,
  // except for the '*' and '&' operators that have to be handled specially
  // by CheckArrayAccess (as there are special cases like &array[arraysize]
  // that are explicitly defined as valid by the standard).
  CheckArrayAccess(Input.get() );

  return Owned(new UnaryOperatorExpr(Input.take(), Opc, resultType, OpLoc) );
}

ExprResult Sematic::analyzeBuiltinCallExpr(ExprResult& fnLhs, ExprVector& args){
  unsigned NumArgs = args.size();
  BuiltinCallExpr* Fn = static_cast<BuiltinCallExpr*>(fnLhs.get() );

  FunctionDecl* FDecl = Fn->getFuncDecl();
  if (FDecl)
    return ExprError();

  unsigned BuiltinID = FDecl->getBuiltinID();
  if (!BuiltinID)
  {
    return ExprError();
  }
  
  // We know the result type of the call, set it.
  Fn->setResultType(FDecl->getCallResultType() );
  // C99 6.5.2.2p7 - the arguments are implicitly converted, as if by
  // assignment, to the types of the corresponding parameter, ...
  unsigned NumArgsInProto = FDecl->getProtoNumArgs();

  if (NumArgs != NumArgsInProto) {
    //errorReport.diagnose(getColumn(), "function arguments don't fit with builtin function definition");
    Diag(diag::err_function_argument_num_unfit);
    //Fn.release();
    return ExprError();
  }
  for (unsigned i = 0; i != NumArgs; i++) {
    ExprNode *Arg = Args[i];

    CheckArrayAccess(Arg);
    ExprResult ArgE = parameterConversion(Owned(Arg), FDecl->getArgType(i) );
    if (ArgE.isInvalid())
      return ExprError();
    
    Fn->setArg(i, ArgE);
  }
  return Fn.move();
}

ExprResult Sematic::parameterConversion(const ExprResult& Arg, const QualType& protoType){
  ExprResult retArg = Arg.move();
  QualType argType = retArg.get()->getQualType();
  CastKind ick = implicitCast(argType, protoType);
  if (ick == kCastIncompatible){
    //diag(argType not suitable for protoType);
    Diag(diag::err_no_argument_conv)<<argType << protoType;
    return ExprError();
  } else if (ick != kCastNoOp){
    return createCastExprByType(retArg, protoType, ick);(sema.cpp)
   }
   return Arg;
}

ExprResult Sematic::createCastExprByType(ExprNode *E, QualType Ty, CastKind Kind){
  if (E->getType() == Ty)
    return Owned(E);
  if (E->getCastKind() == Kind){
    E->setType(Ty);
    return Owned(E);
  }

  return Owned(new CastExpr(Ty, Kind, E) );
}

QualType Sematic::arithIntegerCast(ExprResult& lhsExpr, ExprResult& rhsExpr) {
  QualType lhsTy = lhsExpr.get()->getQualType();
  QualType rhsTy = rhsExpr.get()->getQualType();
  if (lhsTy.isIntegerType() && rhsTy.isIntegerType() ){
    int compare = getIntegerTypeRank(lhsTy, rhsTy);
    if (compare >= 0) {
      rhsExpr = createCastExprByType(rhsExpr.take(), lhsTy, kCastIntegral);
      return lhsTy;
    } else {
      lhsExpr = createCastExprByType(lhsExpr.take(), rhsTy, kCastIntegral);
      return rhsTy;
    }
  }
}

CastKind Sematic::implicitCast(QualType& FromType, const QualType& ToType){
  CastKind ICK = kCastNoOp;
  if (getIntegerTypeRank(FromType, ToType) > 0) {
    // Integral promotion (C++ 4.5).
    ICK = kCastIntegral;
    FromType = ToType;
  } else if (FromType->isFloatingType() &&
    ToType->isIntegralType() ) {
      // Floating-integral conversions (C++ 4.9).
      ICK = kCastFloatingToIntegral;
      FromType = ToType;
  } else if (FromType->isIntegralType() &&
    ToType->isFloatingType() ){
    ICK = kCastIntegralToFloating;
    FromType = ToType;
  } else {
    ICK = kCastIncompatible;
  }
  return ICK;
}

static inline OperatorKind getBinaryOpcode(const TokenKind Kind) {
  OperatorKind Opc;
  switch (Kind) {
  default: assert(0 && "Unknown binop!");
  case Token::star:                 Opc = kBOMul; break;
  case Token::slash:                Opc = kBODiv; break;
  case Token::percent:              Opc = kBORem; break;
  case Token::plus:                 Opc = kBOAdd; break;
  case Token::minus:                Opc = kBOSub; break;
  case Token::lessless:             Opc = kBOShl; break;
  case Token::greatergreater:       Opc = kBOShr; break;
  case Token::lessequal:            Opc = kBOLE; break;
  case Token::less:                 Opc = kBOLT; break;
  case Token::greaterequal:         Opc = kBOGE; break;
  case Token::greater:              Opc = kBOGT; break;
  case Token::exclaimequal:         Opc = kBONE; break;
  case Token::equalequal:           Opc = kBOEQ; break;
  case Token::amp:                  Opc = kBOBitAnd; break;
  case Token::caret:                Opc = kBOBitXor; break;
  case Token::pipe:                 Opc = kBOBitOr; break;
  case Token::ampamp:               Opc = kBOLAnd; break;
  case Token::pipepipe:             Opc = kBOLOr; break;
  case Token::equal:                Opc = kBOAssign; break;
  case Token::comma:                Opc = kBOComma; break;
  }
  return Opc;
}
static inline OperatorKind getUnaryOpcode(TokenKind Kind) {
  OperatorKind Opc;
  switch (Kind) {
  default: assert(false && "Unknown unary op!");
  case Token::plus:         Opc = kUOPlus; break;
  case Token::minus:        Opc = kUOMinus; break;
  case Token::tilde:  
  case Token::exclaim:      Opc = kUOLNot; break;
  }
  return Opc;
}
}
}
