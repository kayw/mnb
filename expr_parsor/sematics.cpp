#include "sematics.h"
#include "symbols.h"
#include "builtinfn.h"

namespace mnb {
namespace expr {

ExprResult Sematic::Owned(ExprNode* E) {
  return ExprResult(E);
}

VarDecl* Sematic::analyzeDeclarator(const Declarator& D) {
  QualType t = getTypeInfoBy(D);
  VarDecl* decl = new VarDecl(t);
  //declGroupMap_.insert(D.getIdentifier(), decl);
  declGroupMap_[D.getIdentifier()] = decl;
  return decl;
}

QualType Sematic::getTypeInfoBy(const Declarator& D) {
  QualType T = D.getDeclType();
  int e = D.getTypeObjects();
  int i = 0;
  for (; e != i ; ++i) {
    const Declarator::DeclaratorChunk& dc = D.getChunckObject(e - i - 1);
    if (dc.ChunkKind == Declarator::DeclaratorChunk::kArrayType) {
      Declarator::DeclaratorChunk::ArrayTypeInfo ati = dc.Arr_;
      T = buildArrayType(T, ati.ElemSize); 
    }
  }
  return T;
}

QualType Sematic::buildArrayType(QualType T, ExprNode* arraySize) {
  //[] arraySize is NULL(size set later as initializer'size)
  //if (!arraySize) {
  //  Diag(diag::err_invalid_array_element_size);
  //  return T;
  //}
  uint64_t constVal = arraySize ? arraySize->evaluate().intVal.uintValue : 0;
  ConstArrayType* pNewCAT = new ConstArrayType(T, constVal);
  return QualType(pNewCAT);
}

ExprResult Sematic::analyzeInitExprs(std::vector<ExprNode*>& initList) {
  //todo initlist type same or enough length set initexprs' type
  if (initList.empty() ) {
    Diag(diag::err_empty_brace_initialiazer);
    return ExprResult(false);
  }
  int ilSize = initList.size();
  QualType formerTy = initList[0]->getQualType();
  // all init element need be equal type
  for (int i = 1; i < ilSize; ++i) {
    if (!initList[i]->getQualType().isSameQualType(formerTy) ) {
      Diag(diag::err_typecheck_initialier_different) << i+1;
      return ExprResult(false);
    }
  }
  return ExprResult(new InitExprs(formerTy, initList) );
}

ExprResult Sematic::analyzeIdentifier(const IdentifierInfo* pII) {
  if (!pII)
    return ExprResult(true);
  BuiltinInfo* pbi = psymbol_table_->findBuiltinIDInfo(pII);
  if (pbi) {
    FunctionDecl* pfd = FunctionDecl::createBuiltinDecl(*pbi, *psymbol_table_);
    return ExprResult(new BuiltinCallExpr(pfd) );
  }
  else if (declGroupMap_.find(pII) != declGroupMap_.end() ) {
    return ExprResult(declGroupMap_[pII]);
  }
  else {
    return ExprResult(true);
  }
}

ExprResult Sematic::analyzeConstantLiteral(Token* pToken) {
  switch(pToken->getKind() ) {
    case Token::numeric:
      //Num* pNumTok = static_cast<Num*>(pToken);
      return ExprResult(new Constant(pToken->getValue(), QualType(psymbol_table_->typeOf(Token::kw_INT) ) ) );
    case Token::real_const:
      //Real* pRealTok = static_cast<Real*>(pToken);
      return ExprResult(new Constant(pToken->getFloatValue(), QualType(psymbol_table_->typeOf(Token::kw_REAL) ) ) );
    case Token::kw_TRUE: case Token::kw_ON:
      return ExprResult(new Constant(true, QualType(psymbol_table_->typeOf(Token::kw_BOOL) ) ) );
    case Token::kw_FALSE: case Token::kw_OFF:
      return ExprResult(new Constant(false, QualType(psymbol_table_->typeOf(Token::kw_BOOL) ) ) );
    default:
      return ExprResult(true);
  }
}

bool Sematic::actOnUninitializedDecl(VarDecl* decl) {
  QualType T = decl->getQualType();
  return decl->setInitialier(T.createDefaultInit(*psymbol_table_) ) == 0;//todo array type
}

ExprResult Sematic::analyzeBinaryOperator(const Token::TokenTag opTokKind, ExprResult& lhs, ExprResult& rhs) {
  OperatorKind opk = getBinaryOpcode(opTokKind);
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
      resultTy = checkShiftOperands(lhs, rhs, opk);
      break;
    case kBOLE: case kBOLT:
    case kBOGE: case kBOGT:
      resultTy = checkCompareOperands(lhs, rhs, true);
      break;
    case kBOEQ: case kBONE:
      resultTy = checkCompareOperands(lhs, rhs, false);
      break;
    case kBOBitAnd: case kBOBitXor: case kBOBitOr:
      resultTy = checkBitwiseOperands(lhs, rhs);
      break;
    case kBOLAnd: case kBOLOr:
      resultTy = checkLogicalOperands(lhs, rhs);
      break;
    default:
      break;
  }

  if (resultTy.isNull() || lhs.isInvalid() || rhs.isInvalid())
    return ExprError();

  // Check for array bounds violations for both sides of the BinaryOperator
  CheckArrayAccess(lhs.get());
  CheckArrayAccess(rhs.get());

  return Owned(new BinaryOperatorExpr(lhs, rhs, opk, resultTy) );
}

QualType Sematic::checkMultiplyDivOperands(ExprResult &lex, ExprResult &rex, bool isDiv) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  if (!lex.get()->getQualType().isArithmeticType() ||
      !rex.get()->getQualType().isArithmeticType())
    return invalidOperands(lex, rex);

  // Check for division by zero.
  if (isDiv && rex.get()->isConstantZero() )
    Diag(diag::warn_division_by_zero);

  return compType;
}

QualType Sematic::checkRemainderOperands(ExprResult& lex, ExprResult& rex) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid() )
    return QualType();

  if (!lex.get()->getQualType().isIntegerType() ||
      !rex.get()->getQualType().isIntegerType() )
    return invalidOperands(lex, rex);

  // Check for remainder by zero.
  if (rex.get()->isConstantZero() )
    Diag(diag::warn_remainder_by_zero);

  return compType;
}

// C99 6.5.6
QualType Sematic::checkAdditionSubtractOperands(ExprResult &lex, ExprResult &rex) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid() )
    return QualType();

  if (!lex.get()->getQualType().isArithmeticType() ||
      !rex.get()->getQualType().isArithmeticType() )
    return invalidOperands(lex, rex);

  return compType;
}

QualType Sematic::checkShiftOperands(ExprResult &lex, ExprResult &rex, unsigned Opc) {
  // Each of the operands shall have integer type.(following C99 6.5.7p2)
  if (!lex.get()->getQualType().isIntegerType() || 
      !rex.get()->getQualType().isIntegerType() )
    return invalidOperands(lex, rex);

  if (lex.isInvalid() )
    return QualType();

  if (rex.isInvalid() )
    return QualType();

  // Sanity-check shift operands
  diagnoseBadShiftValues(lex, rex, Opc);

  // The type of the result is that of the promoted left operand.
  return arithmeticConversion(lex, rex);
}

QualType Sematic::checkCompareOperands(ExprResult &lex, ExprResult &rex, bool isRelational) {
  QualType lType = lex.get()->getQualType();
  QualType rType = rex.get()->getQualType();

  if (lex.get()->getQualType().isArithmeticType()
      && rex.get()->getQualType().isArithmeticType()
      && isRelational) {
    arithmeticConversion(lex, rex);
    if (lex.isInvalid() || rex.isInvalid() )
      return QualType();
  }else if (!isRelational /* == !=*/
      && lex.get()->getQualType().hasIntegerRepresentation()
      && rex.get()->getQualType().hasIntegerRepresentation() ) {
    //lex = unaryConversions(lex.take());
    //if (lex.isInvalid())
    //  return QualType();

    //rex = unaryConversions(rex.take());
    //if (rex.isInvalid())
    //  return QualType();
    arithIntegerCast(lex, rex);
  }

  // The result of comparisons is 'bool' in C++, 'int' in C. (for refrence, here is BOOL type)
  QualType ResultTy = QualType(psymbol_table_->typeOf(Token::kw_BOOL) );
  return ResultTy;
}

QualType Sematic::checkBitwiseOperands(ExprResult &lex, ExprResult &rex) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();
  if (lex.get()->getQualType().isIntegerType() && 
      rex.get()->getQualType().isIntegerType() )
    return compType;

  return invalidOperands(lex, rex);
}

QualType Sematic::checkLogicalOperands(ExprResult &lex, ExprResult &rex) {
  // Diagnose cases where the user write a logical and/or but probably meant a
  // bitwise one.  We do this when the LHS is a non-bool integer and the RHS
  // is a constant.
  if (!lex.get()->getQualType().get()->isBooleanType() ||
      !rex.get()->getQualType().get()->isBooleanType())
    return invalidOperands(lex, rex);

  return QualType(psymbol_table_->typeOf(Token::kw_BOOL));
}

QualType Sematic::checkAssignmentOperands(ExprNode* LHS, ExprResult &RHS) {
  // Verify that LHS is a modifiable lvalue, and emit error if not.
  QualType LHSType = LHS->getQualType();//ptype null
  QualType RHSType = RHS.get()->getQualType();

  // just comparing types.
  // Common case: no conversion required.
  if (LHSType.isSameQualType(RHSType) ) {
    return LHSType;
  }
  CastKind Kind =  kCastNoOp;
  switch(LHSType.get()->getKind() ) {
    case kBoolTy:
      if (RHSType.isIntegerType() )
        Kind = kCastIntegralToBoolean;
      if (RHSType.get()->isFloatingType() )
        Kind = kCastFloatingToBoolean;
      break;
    case kIntTy:
      if (RHSType.get()->isBooleanType() || RHSType.isIntegerType() )
        Kind = kCastIntegral;
      if (RHSType.get()->isFloatingType() )
        Kind = kCastFloatingToIntegral;
      break;
    case kFloatTy:
      if (RHSType.isIntegerType() || RHSType.get()->isBooleanType() )
        Kind = kCastIntegralToFloating;
      break;
    default:
      break;
  }
  // C99 6.5.16.1p2: The value of the right operand is converted to the
  // type of the assignment expression.
  // CheckAssignmentConstraints allows the left-hand side to be a reference,
  // so that we can use references in built-in functions even in C.
  // The getNonReferenceType() call makes sure that the resulting expression
  // does not have reference type.
  RHS = createCastExprByType(RHS.get(), LHSType, Kind);
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

int Sematic::getIntegerTypeRank(const QualType& lhsTy, const QualType& rhsTy) {
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

QualType Sematic::arithmeticConversion(ExprResult& lhsExpr, ExprResult& rhsExpr) {
  QualType lhsTy = lhsExpr.get()->getQualType();//todo pType null return other type constant node type null
  QualType rhsTy = rhsExpr.get()->getQualType();
  if (lhsTy.isSameQualType(rhsTy) )
    return lhsTy;
  if (lhsTy.isIntegerType() && rhsTy.isIntegerType() )
    return arithIntegerCast(lhsExpr, rhsExpr);
  bool lhsFloat = lhsTy.get()->isFloatingType();
  bool rhsFloat = rhsTy.get()->isFloatingType();
  // If we have an integer operand, the result is the real floating type.
  if (lhsFloat) {
    if (rhsTy.isIntegerType()) {
      // Convert rhs to the lhs floating point type.
      rhsExpr = createCastExprByType(rhsExpr.get(), lhsTy, kCastIntegralToFloating);
      return lhsTy;
    }
  }
  if (rhsFloat) {
    if (lhsTy.isIntegerType()) {
      // Convert lhs to the rhs floating point type.
      lhsExpr = createCastExprByType(lhsExpr.get(), rhsTy, kCastIntegralToFloating);
      return rhsTy;
    }
  }
  return lhsTy;
}

QualType Sematic::invalidOperands(ExprResult &lex, ExprResult &rex) {
  Diag(diag::err_typecheck_invalid_operands)
    << lex.get()->getQualType().get()->getTypeLiteral()
    << rex.get()->getQualType().get()->getTypeLiteral();
  return QualType();
}

void Sematic::diagnoseBadShiftValues(ExprResult &lex, ExprResult &rex, unsigned Opc) {
  ExprValue eval = rex.get()->evaluate();
  if (eval.isNegative() ) {
    Diag(diag::warn_shift_negative);
    return;
  }
  //llvm::APInt LeftBits(Right.getBitWidth(),
  //                     S.Context.getTypeSize(lex.get()->getType()));
  int32_t leftbits = lex.get()->getQualType().get()->getTypeWidth();
  if (static_cast<int32_t>(eval.intVal.uintValue) > leftbits){//Right.uge(LeftBits)) {
    Diag(diag::warn_shift_gt_typewidth);
    return;
  }
  if (Opc != kBOShl)
    return;

  // For the moment, ExprValue support the 32 bit operands' calculation, We don't pass type width for the ExprValue.
  // So here we warn signed or unsigned integer shift operation result exceed the type width.
  // In some stage we should cycle the bits in ExprValue.shl for the unsigned operand and stop calculate for the signed.
  QualType LHSTy = lex.get()->getQualType();
  if (!LHSTy.isIntegerType() )//|| !LHSTy.isSignedInteger() )
    return;
  ExprValue leftvalue = lex.get()->evaluate();
  int32_t resultBits = eval.intVal.uintValue + leftvalue.getValidBits();
  if (leftbits < resultBits) //LeftBits.uge(ResultBits))
    Diag(diag::warn_shift_result_gt_typewidth) << resultBits 
      << LHSTy.get()->getTypeLiteral() << leftbits;
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
  if(expr->getExprClass() != ExprNode::kArraySubscriptExprClasss )
    return;
  const ExprNode* BaseExpr = expr->getBase();
  ExprNode* IndexExpr = expr->getIdx();

  const ConstArrayType *ArrayTy =
    static_cast<ConstArrayType*>(BaseExpr->getQualType().get() );
  if (!ArrayTy)
    return;

  QualType indexType = IndexExpr->getQualType();
  if (!indexType.isIntegerType() )
    return;
  ExprValue index = IndexExpr->evaluate();

  if (!index.isNegative() ) {
    int32_t size = ArrayTy->getArraySize();
    if (size <= 0)
      return;
    // For array subscripting the index must be less than size, but for pointer
    // arithmetic also allow the index (offset) to be equal to size since
    // computing the next address after the end of the array is legal and
    // commonly done e.g. in C++ iterators and range-based for loops.
    if (static_cast<int32_t>(index.intVal.uintValue) >= size)
      Diag(diag::warn_array_index_exceeds_bounds) << index.intVal.uintValue//index.toString(10, true)
                          << size;
                          //<< (unsigned)(~0U);//(unsigned)size.getLimitedValue(~0U);
  } else {
    Diag(diag::warn_array_index_precedes_bounds)<< index.intVal.uintValue;//toString(10, true);
  }

}

ExprResult Sematic::analyzeArraySubscript(ExprResult base, ExprResult idx) {
  ExprNode* lhsExp = base.get();
  ExprNode* rhsExp = idx.get();

  QualType lhsTy = lhsExp->getQualType(), rhsTy = rhsExp->getQualType();

  // C99 6.5.2.1p2: the expression e1[e2] is by definition precisely equivalent
  // to the expression *((e1)+(e2)). This means the array "Base" may actually be
  // in the subscript position. As a result, we need to derive the array base
  // and index from the expression types.
  QualType resultType;
  if (lhsTy.isArrayElement()) {
    // C99 6.5.2.1p1
    if(!rhsTy.isIntegerType() ) {
      Diag(diag::err_typecheck_subscript_not_integer);
      return ExprError();
    }
    resultType = lhsExp->getQualType();
  } else {
    Diag(diag::err_typecheck_subscript_value);
    return ExprError();
  }
  return Owned(new ArraySubscriptExpr(lhsExp, rhsExp, resultType));
}

ExprResult Sematic::analyzeUnaryOperator(Token::TokenTag Kind, ExprNode* InputExpr) {
  OperatorKind Opc =  getUnaryOpcode(Kind);
  ExprResult Input = Owned(InputExpr);
  QualType resultType;
  switch (Opc) {
  case kUOPlus:
  case kUOMinus:
    {
    //Input = unaryConversions(Input.take());
      if (Input.isInvalid()) return ExprError();
      resultType = Input.get()->getQualType();
      if (resultType.isArithmeticType() ) 
        break;

      Diag(diag::err_typecheck_unary_expr) << resultType.get()->getTypeLiteral();
      return ExprError();
    }
    //return ExprError(Diag(OpLoc, diag::err_typecheck_unary_expr)
    //  << resultType << Input.get()->getSourceRange());

  case kUOLNot: // logical negation
    {
    // Unlike +/-/~, integer promotions aren't done here (C99 6.5.3.3p5).
    //Input = DefaultFunctionArrayLvalueConversion(Input.take());
    //if (Input.isInvalid()) return ExprError();
      resultType = Input.get()->getQualType();
      if (!resultType.get()->isBooleanType() ) {
        // C99 6.5.3.3p1: ok, fallthrough;
        Diag(diag::err_typecheck_unary_expr) << resultType.get()->getTypeLiteral();
        return ExprError();
      }
    }
    
    // LNot always has type int. C99 6.5.3.3p5.
    // In C++, it's bool. C++ 5.3.1p8
    //resultType = Context.getLogicalOperationType();
    break;
  default:
    break;
  }

  if (resultType.isNull() || Input.isInvalid())
    return ExprError();

  // Check for array bounds violations in the operand of the UnaryOperator,
  // except for the '*' and '&' operators that have to be handled specially
  // by CheckArrayAccess (as there are special cases like &array[arraysize]
  // that are explicitly defined as valid by the standard).
  CheckArrayAccess(Input.get() );

  return Owned(new UnaryOperatorExpr(Input.get(), Opc, resultType) );
}

ExprResult Sematic::analyzeBuiltinCallExpr(ExprResult& fnLhs, ExprVector& args) {
  unsigned NumArgs = args.size();
  BuiltinCallExpr* Fn = static_cast<BuiltinCallExpr*>(fnLhs.get() );

  FunctionDecl* FDecl = Fn->getFuncDecl();
  if (!FDecl)
    return ExprError();

  //unsigned BuiltinID = FDecl->getBuiltinID();
  //if (!BuiltinID)
  //{
  //  return ExprError();
  //}
  //
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
    ExprNode *Arg = args[i].get();

    CheckArrayAccess(Arg);
    ExprResult ArgE = parameterConversion(Owned(Arg), FDecl->getArgType(i) );
    if (ArgE.isInvalid())
      return ExprError();
    
    Fn->setArg(i, ArgE);
  }
  return ExprResult(Fn);
}

ExprResult Sematic::parameterConversion(const ExprResult& Arg, const QualType& protoType) {
  QualType argType = Arg.get()->getQualType();
  if (!argType.isSameQualType(protoType) ) {
    CastKind ick = implicitCast(argType, protoType);
    if (ick == kCastIncompatible) {
      //diag(argType not suitable for protoType);
      Diag(diag::err_no_argument_conv) << argType.get()->getTypeLiteral() << protoType.get()->getTypeLiteral();
      return ExprError();
    } else if (ick != kCastNoOp) {
      return createCastExprByType(Arg.get(), protoType, ick);//(sema.cpp)
    }
  }
  return Arg;
}

ExprResult Sematic::createCastExprByType(ExprNode *E, QualType Ty, CastKind Kind){
  if (E->getQualType().isSameQualType(Ty) )
    return Owned(E);
  if (E->getCastKind() == Kind) {
    E->setResultType(Ty);
    return Owned(E);
  }

  return Owned(new CastExpr(Ty, Kind, ExprResult(E) ) );
}

QualType Sematic::arithIntegerCast(ExprResult& lhsExpr, ExprResult& rhsExpr) {
  QualType lhsTy = lhsExpr.get()->getQualType();
  QualType rhsTy = rhsExpr.get()->getQualType();
  if (lhsTy.isIntegerType() && rhsTy.isIntegerType() ){
    int compare = getIntegerTypeRank(lhsTy, rhsTy);
    if (compare >= 0) {
      rhsExpr = createCastExprByType(rhsExpr.get(), lhsTy, kCastIntegral);
      return lhsTy;
    } else {
      lhsExpr = createCastExprByType(lhsExpr.get(), rhsTy, kCastIntegral);
      return rhsTy;
    }
  }
  return lhsTy;
}

CastKind Sematic::implicitCast(QualType& fromType, const QualType& toType) {
  CastKind ICK = kCastNoOp;
  //if (getIntegerTypeRank(fromType, toType) > 0) {
    // Integral promotion (C++ 4.5).
  if (fromType.isIntegerType() && toType.isIntegerType() ) {
    ICK = kCastIntegral;
    fromType = toType;
  } else if (fromType.get()->isFloatingType() &&
    toType.isIntegerType() ) {
      // Floating-integral conversions (C++ 4.9).
      ICK = kCastFloatingToIntegral;
      fromType = toType;
  } else if (fromType.isIntegerType() &&
    toType.get()->isFloatingType() ){
    ICK = kCastIntegralToFloating;
    fromType = toType;
  } else {
    ICK = kCastIncompatible;
  }
  return ICK;
}

OperatorKind Sematic::getBinaryOpcode(const Token::TokenTag Kind) {
  OperatorKind Opc;
  switch (Kind) {
  default: assert(false && "Unknown binop!");
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

OperatorKind Sematic::getUnaryOpcode(const Token::TokenTag Kind) {
  OperatorKind Opc;
  switch (Kind) {
  default: assert(false && "Unknown unary op!");
  case Token::plus:         Opc = kUOPlus; break;
  case Token::minus:        Opc = kUOMinus; break;
  case Token::tilde:  Opc = kUOLNot; break;
  //case Token::exclaim:      
  }
  return Opc;
}

}
}
