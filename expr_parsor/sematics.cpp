namespace mnb{
namespace expr{
static inline UnaryOperatorKind ConvertTokenKindToUnaryOpcode(
  tok::TokenKind Kind) {
  UnaryOperatorKind Opc;
  switch (Kind) {
  default: assert(0 && "Unknown unary op!");
  case tok::plus:         Opc = UO_Plus; break;
  case tok::minus:        Opc = UO_Minus; break;
  case tok::tilde:        Opc = UO_Not; break;
  case tok::exclaim:      Opc = UO_LNot; break;
  }
  return Opc;
}
VarDecl* Sematic::analyzeDeclartor(const Declarator& D){
  QualType t = getTypeInfoBy(D);
  VarDecl* decl = new VarDecl(t);
  return decl;
}

QualType Sematic::getTypeInfoBy(const Declarator& D){
  QualType T = D.getDeclType();
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
    //diag non variable size array not supported
    return T;
  }
  uint64_t constVal = arraySize.evaluate();
  ConstArrayType* pNewCAT = new ConstArrayType(T, constVal);
  return QualType(pNewCAT);
}

ExprResult Sematic::analyzeInitExprs(const std::vector<ExprNode>& initList){
  return ExprResult(new InitExprs(initList) );
}
void Sematic::actOnUninitializedDecl(VarDecl* decl){
  QualType T = decl.getQualType();
  decl->setInitialier(T.createDefaultInit() );
}

ExprResult Sematic::analyzeBinaryOperator(const Token opTok, ExprResult lhs, ExprResult rhs){
  BinaryOperatorKind opk = getBinaryOperKind(opTok);
  QualType resultTy;
  switch(opk){
    case kBOAssign:
      resultTy = checkAssignmentOperands(lhs, rhs);
      break;
    case kBOMul: 
      resultTy = checkMultiplyDivRemainderOperands(lhs, rhs, false);
      break;
    case kBORem: case kBODiv:
      resultTy = checkMultiplyDivRemainderOperands(lhs, rhs, true);
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

  return Owned(new (Context) BinaryOperator(lhs.take(), rhs.take(), Opc,
                                              ResultTy, VK, OK, OpLoc));
}

QualType Sematic::checkMultiplyDivRemainderOperands(
  ExprResult &lex, ExprResult &rex, SourceLocation Loc, bool isDiv) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  if (!lex.get()->getType()->isIntegerType() ||
      !rex.get()->getType()->isIntegerType())
    return InvalidOperands(Loc, lex, rex);

  // Check for remainder by zero.
  if (isDiv && rex.get()->isNullPointerConstant(Context,
                                       Expr::NPC_ValueDependentIsNotNull))
    DiagRuntimeBehavior(Loc, rex.get(), PDiag(diag::warn_remainder_by_zero)
                                 << rex.get()->getSourceRange());

  return compType;
}
QualType Sematic::checkAdditionSubtractOperands( // C99 6.5.6
  ExprResult &lex, ExprResult &rex, SourceLocation Loc) {
  QualType compType = arithmeticConversion(lex, rex);
  if (lex.isInvalid() || rex.isInvalid())
    return QualType();

  // handle the common case first (both operands are arithmetic).
  if (lex.get()->getType()->isArithmeticType() &&
      rex.get()->getType()->isArithmeticType()) {
    return compType;
  }

  Expr* PExp = lex.get(), *IExp = rex.get();

  DiagNonBoolType();
  if (PExp.getType().isBooleanType() || IExp.getType().isBooleanType() ) {
    Diag("add on boolean type");
    return InvalidOperands();
  }
  return InvalidOperands(Loc, lex, rex);
}

QualType Sematic::checkShiftOperands(ExprResult &lex, ExprResult &rex,
                                  SourceLocation Loc, unsigned Opc) {
  // C99 6.5.7p2: Each of the operands shall have integer type.
  if (!lex.get()->getType()->hasIntegerRepresentation() || 
      !rex.get()->getType()->hasIntegerRepresentation())
    return InvalidOperands(Loc, lex, rex);

  // Shifts don't perform usual arithmetic conversions, they just do integer
  // promotions on each operand. C99 6.5.7p3

  // For the LHS, do usual unary conversions, but then reset them away
  // if this is a compound assignment.
  ExprResult old_lex = lex;
  lex = unaryConversions(lex.take());
  if (lex.isInvalid())
    return QualType();
  QualType LHSTy = lex.get()->getType();

  // The RHS is simpler.
  rex = unaryConversions(rex.take());
  if (rex.isInvalid())
    return QualType();

  // Sanity-check shift operands
  DiagnoseBadShiftValues(*this, lex, rex, Loc, Opc, LHSTy);

  // "The type of the result is that of the promoted left operand."
  return LHSTy;
}
QualType Sematic::checkCompareOperands(ExprResult &lex, ExprResult &rex,
                                    SourceLocation Loc, unsigned OpaqueOpc,
                                    bool isRelational) {
  BinaryOperatorKind Opc = (BinaryOperatorKind) OpaqueOpc;

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
  }else if (!isRelational
      && lex.get()->getType()->isIntegerBoolType()
      && rex.get()->getType()->isIntegerBoolType()){
    lex = unaryConversions(lex.take());
    if (lex.isInvalid())
      return QualType();

    rex = unaryConversions(rex.take());
    if (rex.isInvalid())
      return QualType();
  }

  // The result of comparisons is 'bool' in C++, 'int' in C.
  QualType ResultTy = BoolTy;
  return ResultTy;

  //if (isRelational) {
  //  if (lType->isArithmeticType() && rType->isArithmeticType())
  //    return ResultTy;
  //} else {
  //  // Check for comparisons of floating point operands using != and ==.
  //  //if (lType->hasFloatingRepresentation())
  //  //  CheckFloatComparison(Loc, lex.get(), rex.get());

  //}

  //return InvalidOperands(Loc, lex, rex);
}

QualType Sematic::checkBitwiseOperands(
  ExprResult &lex, ExprResult &rex, SourceLocation Loc, bool isCompAssign) {
  ExprResult lexResult = Owned(lex), rexResult = Owned(rex);
  QualType compType = arithmeticConversion(lexResult, rexResult);
  if (lexResult.isInvalid() || rexResult.isInvalid())
    return QualType();
  lex = lexResult.take();
  rex = rexResult.take();
  if (lex.get().getType().isIntegerType() && 
      rex.get().getType().isIntegerType() )
    return compType;

  return InvalidOperands(Loc, lex, rex);
}

inline QualType Sematic::checkLogicalOperands( // C99 6.5.[13,14]
  ExprResult &lex, ExprResult &rex, SourceLocation Loc, unsigned Opc) {
  
  // Diagnose cases where the user write a logical and/or but probably meant a
  // bitwise one.  We do this when the LHS is a non-bool integer and the RHS
  // is a constant.
  if (lex.get()->getType()->isIntegerType() &&
      !lex.get()->getType()->isBooleanType() &&
      rex.get()->getType()->isIntegerType() ) {
    // If the RHS can be constant folded, and if it constant folds to something
    // that isn't 0 or 1 (which indicate a potential logical operation that
    // happened to fold to true/false) then warn.
    // Parens on the RHS are ignored.
    Expr::EvalResult Result;
    if (rex.get()->Evaluate(Result, Context) && !Result.HasSideEffects)
      if ((getLangOptions().Bool && !rex.get()->getType()->isBooleanType()) ||
          (Result.Val.getInt() != 0 && Result.Val.getInt() != 1)) {
        Diag(Loc, diag::warn_logical_instead_of_bitwise)
          << rex.get()->getSourceRange()
          << (Opc == BO_LAnd ? "&&" : "||");
        // Suggest replacing the logical operator with the bitwise version
        Diag(Loc, diag::note_logical_instead_of_bitwise_change_operator)
            << (Opc == BO_LAnd ? "&" : "|")
            << FixItHint::CreateReplacement(SourceRange(
                Loc, Lexer::getLocForEndOfToken(Loc, 0, getSourceManager(),
                                                getLangOptions())),
                                            Opc == BO_LAnd ? "&" : "|");
        if (Opc == BO_LAnd)
          // Suggest replacing "Foo() && kNonZero" with "Foo()"
          Diag(Loc, diag::note_logical_instead_of_bitwise_remove_constant)
              << FixItHint::CreateRemoval(
                  SourceRange(
                      Lexer::getLocForEndOfToken(lex.get()->getLocEnd(),
                                                 0, getSourceManager(),
                                                 getLangOptions()),
                      rex.get()->getLocEnd()));
      }
  }
  
  lex = unaryConversions(lex.take());
  if (lex.isInvalid())
    return QualType();

  rex = unaryConversions(rex.take());
  if (rex.isInvalid())
    return QualType();

  if (!lex.get()->getType()->isScalarType() ||
      !rex.get()->getType()->isScalarType())
    return InvalidOperands(Loc, lex, rex);

  return Context.IntTy;

  // The following is safe because we only use this method for
  // non-overloadable operands.

  // C++ [expr.log.and]p1
  // C++ [expr.log.or]p1
  // The operands are both contextually converted to type bool.
  ExprResult lexRes = convertToBoolean()//PerformContextuallyConvertToBool(lex.get());
  if (lexRes.isInvalid())
    return InvalidOperands(Loc, lex, rex);
  lex = move(lexRes);

  ExprResult rexRes = PerformContextuallyConvertToBool(rex.get());
  if (rexRes.isInvalid())
    return InvalidOperands(Loc, lex, rex);
  rex = move(rexRes);

  // C++ [expr.log.and]p2
  // C++ [expr.log.or]p2
  // The result is a bool.
  return Context.BoolTy;
}
QualType Sematic::checkAssignmentOperands(Expr *LHS, ExprResult &RHS,
                                       SourceLocation Loc) {
  // Verify that LHS is a modifiable lvalue, and emit error if not.
  if (LHS->isModifiableLvalue(this) )//CheckForModifiableLvalue(LHS, Loc, *this))
    return QualType();

  QualType LHSType = LHS->getType();
  QualType RHSType =RHS.get()->getType();
  AssignConvertType ConvTy;
  QualType LHSTy(LHSType);
  // Simple assignment "x = y".
  ConvTy = CheckSingleAssignmentConstraints(LHSTy, RHS);
  if (RHS.isInvalid())
    return QualType();

  if (DiagnoseAssignmentResult(ConvTy, Loc, LHSType, RHSType,
                               RHS.get(), AA_Assigning))
    return QualType();

  // C99 6.5.16p3: The type of an assignment expression is the type of the
  // left operand unless the left operand has qualified type, in which case
  // it is the unqualified version of the type of the left operand.
  // C99 6.5.16.1p2: In simple assignment, the value of the right operand
  // is converted to the type of the assignment expression (above).
  // C++ 5.17p1: the type of the assignment expression is that of its left
  // operand.
  return (getLangOptions().CPlusPlus
          ? LHSType : LHSType.getUnqualifiedType());
}
Sematic::AssignConvertType
Sematic::CheckSingleAssignmentConstraints(QualType lhsType, ExprResult &rExpr) {
  QualType rhsType = rhs.get()->getType();
  QualType origLhsType = lhsType;

  // Get canonical types.  We're not formatting these types, just comparing
  // them.
  lhsType = Context.getCanonicalType(lhsType).getUnqualifiedType();
  rhsType = Context.getCanonicalType(rhsType).getUnqualifiedType();

  // Common case: no conversion required.
  if (lhsType == rhsType) {
    Kind = CK_NoOp;
    return Compatible;
  }
  if (lhsType.isBooleanType() )
    Kind = ck_inttobool
    Kind = ck_FloatToBool;
}
bool Sematic::DiagnoseAssignmentResult(AssignConvertType ConvTy,
                                    SourceLocation Loc,
                                    QualType DstType, QualType SrcType,
                                    Expr *SrcExpr, AssignmentAction Action) {
  // Decode the result (notice that AST's are still created for extensions).
  bool isInvalid = false;
  unsigned DiagKind;
  FixItHint Hint;
  ConversionFixItGenerator ConvHints;
  bool MayHaveConvFixit = false;

  switch (ConvTy) {
  default: assert(0 && "Unknown conversion type");
  case Compatible: return false;
  case PointerToInt:
    DiagKind = diag::ext_typecheck_convert_pointer_int;
    ConvHints.tryToFixConversion(SrcExpr, SrcType, DstType, *this);
    MayHaveConvFixit = true;
    break;
  case IntToPointer:
    DiagKind = diag::ext_typecheck_convert_int_pointer;
    ConvHints.tryToFixConversion(SrcExpr, SrcType, DstType, *this);
    MayHaveConvFixit = true;
    break;
  case IncompatiblePointer:
    MakeObjCStringLiteralFixItHint(*this, DstType, SrcExpr, Hint);
    DiagKind = diag::ext_typecheck_convert_incompatible_pointer;
    if (Hint.isNull() && !CheckInferredResultType) {
      ConvHints.tryToFixConversion(SrcExpr, SrcType, DstType, *this);
    }
    MayHaveConvFixit = true;
    break;
  case IncompatiblePointerSign:
    DiagKind = diag::ext_typecheck_convert_incompatible_pointer_sign;
    break;
  case FunctionVoidPointer:
    DiagKind = diag::ext_typecheck_convert_pointer_void_func;
    break;
  case IncompatiblePointerDiscardsQualifiers: {
    // Perform array-to-pointer decay if necessary.
    if (SrcType->isArrayType()) SrcType = Context.getArrayDecayedType(SrcType);

    Qualifiers lhq = SrcType->getPointeeType().getQualifiers();
    Qualifiers rhq = DstType->getPointeeType().getQualifiers();
    if (lhq.getAddressSpace() != rhq.getAddressSpace()) {
      DiagKind = diag::err_typecheck_incompatible_address_space;
      break;


    } else if (lhq.getObjCLifetime() != rhq.getObjCLifetime()) {
      DiagKind = diag::err_typecheck_incompatible_ownership;
      break;
    }

    llvm_unreachable("unknown error case for discarding qualifiers!");
    // fallthrough
  }
  case CompatiblePointerDiscardsQualifiers:
    // If the qualifiers lost were because we were applying the
    // (deprecated) C++ conversion from a string literal to a char*
    // (or wchar_t*), then there was no error (C++ 4.2p2).  FIXME:
    // Ideally, this check would be performed in
    // checkPointerTypesForAssignment. However, that would require a
    // bit of refactoring (so that the second argument is an
    // expression, rather than a type), which should be done as part
    // of a larger effort to fix checkPointerTypesForAssignment for
    // C++ semantics.
    if (getLangOptions().CPlusPlus &&
        IsStringLiteralToNonConstPointerConversion(SrcExpr, DstType))
      return false;
    DiagKind = diag::ext_typecheck_convert_discards_qualifiers;
    break;
  case IncompatibleNestedPointerQualifiers:
    DiagKind = diag::ext_nested_pointer_qualifier_mismatch;
    break;
  case IntToBlockPointer:
    DiagKind = diag::err_int_to_block_pointer;
    break;
  case IncompatibleBlockPointer:
    DiagKind = diag::err_typecheck_convert_incompatible_block_pointer;
    break;
  case IncompatibleObjCQualifiedId:
    // FIXME: Diagnose the problem in ObjCQualifiedIdTypesAreCompatible, since
    // it can give a more specific diagnostic.
    DiagKind = diag::warn_incompatible_qualified_id;
    break;
  case IncompatibleVectors:
    DiagKind = diag::warn_incompatible_vectors;
    break;
  case IncompatibleObjCWeakRef:
    DiagKind = diag::err_arc_weak_unavailable_assign;
    break;
  case Incompatible:
    DiagKind = diag::err_typecheck_convert_incompatible;
    ConvHints.tryToFixConversion(SrcExpr, SrcType, DstType, *this);
    MayHaveConvFixit = true;
    isInvalid = true;
    break;
  }

  QualType FirstType, SecondType;
  switch (Action) {
  case AA_Assigning:
  case AA_Initializing:
    // The destination type comes first.
    FirstType = DstType;
    SecondType = SrcType;
    break;

  case AA_Returning:
  case AA_Passing:
  case AA_Converting:
  case AA_Sending:
  case AA_Casting:
    // The source type comes first.
    FirstType = SrcType;
    SecondType = DstType;
    break;
  }

  PartialDiagnostic FDiag = PDiag(DiagKind);
  FDiag << FirstType << SecondType << Action << SrcExpr->getSourceRange();

  // If we can fix the conversion, suggest the FixIts.
  assert(ConvHints.isNull() || Hint.isNull());
  if (!ConvHints.isNull()) {
    for (llvm::SmallVector<FixItHint, 1>::iterator
        HI = ConvHints.Hints.begin(), HE = ConvHints.Hints.end();
        HI != HE; ++HI)
      FDiag << *HI;
  } else {
    FDiag << Hint;
  }
  if (MayHaveConvFixit) { FDiag << (unsigned) (ConvHints.Kind); }

  Diag(Loc, FDiag);

  return isInvalid;
}
QualType Sematic::arithmeticConversion(ExprResult& lhsExpr, ExprResult& rhsExpr){
  lhsExpr = unaryConversios(lhsExpr.take() );
    if (lhsExpr.isInvalid())
      return QualType();
  rhsExpr = unaryConversions(rhsExpr.take() );
  if (rhsExpr.isInvalid())
    return QualType();
  QualType lhsTy = lhsExpr.get()->getQualType();
  QualType rhsTy = rhsExpr.get()->getQualType();
  if (lhsTy == rhsTy)
    return lhsTy;
  int compare = Context.getIntegerTypeOrder(lhs, rhs);
  bool lhsSigned = lhs->hasSignedIntegerRepresentation(),
       rhsSigned = rhs->hasSignedIntegerRepresentation();
  if (lhsSigned == rhsSigned) {
    // Same signedness; use the higher-ranked type
    if (compare >= 0) {
      rhsExpr = impCastExprToType(rhsExpr.take(), lhs, CK_IntegralCast);
      return lhs;
    } else
      lhsExpr = impCastExprToType(lhsExpr.take(), rhs, CK_IntegralCast);
    return rhs;
  } else if (compare != (lhsSigned ? 1 : -1)) {
    // The unsigned type has greater than or equal rank to the
    // signed type, so use the unsigned type
    if (rhsSigned) {
      rhsExpr = impCastExprToType(rhsExpr.take(), lhs, CK_IntegralCast);
      return lhs;
    } else
      lhsExpr = impCastExprToType(lhsExpr.take(), rhs, CK_IntegralCast);
    return rhs;
  } else if (Context.getIntWidth(lhs) != Context.getIntWidth(rhs)) {
    // The two types are different widths; if we are here, that
    // means the signed type is larger than the unsigned type, so
    // use the signed type.
    if (lhsSigned) {
      rhsExpr = impCastExprToType(rhsExpr.take(), lhs, CK_IntegralCast);
      return lhs;
    } else
      lhsExpr = impCastExprToType(lhsExpr.take(), rhs, CK_IntegralCast);
    return rhs;
  } else {
    // The signed type is higher-ranked than the unsigned type,
    // but isn't actually any bigger (like unsigned int and long
    // on most 32-bit systems).  Use the unsigned type corresponding
    // to the signed type.
    QualType result =
      Context.getCorrespondingUnsignedType(lhsSigned ? lhs : rhs);
    rhsExpr = impCastExprToType(rhsExpr.take(), result, CK_IntegralCast);
    lhsExpr = impCastExprToType(lhsExpr.take(), result, CK_IntegralCast);
    return result;
  }
}

ExprResult Sematic::unaryConversions(Expr* E){
  QualType Ty = E->getQualType();
  if (Ty->isPromotableIntegerType() ) {
      QualType PT = Context.getPromotedIntegerType(Ty);
      E = impCastExprToType(E, PT, CK_IntegralCast).take();
      return Owned(E);
  }
}
/// \brief Emit a diagnostic that describes an effect on the run-time behavior
/// of the program being compiled.
///
/// This routine emits the given diagnostic when the code currently being
/// type-checked is "potentially evaluated", meaning that there is a
/// possibility that the code will actually be executable. Code in sizeof()
/// expressions, code used only during overload resolution, etc., are not
/// potentially evaluated. This routine will suppress such diagnostics or,
/// in the absolutely nutty case of potentially potentially evaluated
/// expressions (C++ typeid), queue the diagnostic to potentially emit it
/// later.
///
/// This routine should be used for all diagnostics that describe the run-time
/// behavior of a program, such as passing a non-POD value through an ellipsis.
/// Failure to do so will likely result in spurious diagnostics or failures
/// during overload resolution or within sizeof/alignof/typeof/typeid.
bool Sema::DiagRuntimeBehavior(SourceLocation Loc, const Stmt *stmt,
                               const PartialDiagnostic &PD) {
  switch (ExprEvalContexts.back().Context) {
  case Unevaluated:
    // The argument will never be evaluated, so don't complain.
    break;

  case PotentiallyEvaluated:
  case PotentiallyEvaluatedIfUsed:
      Diag(Loc, PD);
      
    return true;

  case PotentiallyPotentiallyEvaluated:
    ExprEvalContexts.back().addDiagnostic(Loc, PD);
    break;
  }

  return false;
}
static void DiagnoseBadShiftValues(Sema& S, ExprResult &lex, ExprResult &rex,
                                   SourceLocation Loc, unsigned Opc,
                                   QualType LHSTy) {
  llvm::APSInt Right;
  // Check right/shifter operand
  if (rex.get()->isValueDependent() ||
      !rex.get()->isIntegerConstantExpr(Right, S.Context))
    return;

  if (Right.isNegative()) {
    S.DiagRuntimeBehavior(Loc, rex.get(),
                          S.PDiag(diag::warn_shift_negative)
                            << rex.get()->getSourceRange());
    return;
  }
  llvm::APInt LeftBits(Right.getBitWidth(),
                       S.Context.getTypeSize(lex.get()->getType()));
  if (Right.uge(LeftBits)) {
    S.DiagRuntimeBehavior(Loc, rex.get(),
                          S.PDiag(diag::warn_shift_gt_typewidth)
                            << rex.get()->getSourceRange());
    return;
  }
  if (Opc != BO_Shl)
    return;

  // When left shifting an ICE which is signed, we can check for overflow which
  // according to C++ has undefined behavior ([expr.shift] 5.8/2). Unsigned
  // integers have defined behavior modulo one more than the maximum value
  // representable in the result type, so never warn for those.
  llvm::APSInt Left;
  if (lex.get()->isValueDependent() ||
      !lex.get()->isIntegerConstantExpr(Left, S.Context) ||
      LHSTy->hasUnsignedIntegerRepresentation())
    return;
  llvm::APInt ResultBits =
      static_cast<llvm::APInt&>(Right) + Left.getMinSignedBits();
  if (LeftBits.uge(ResultBits))
    return;
  llvm::APSInt Result = Left.extend(ResultBits.getLimitedValue());
  Result = Result.shl(Right);

  // Print the bit representation of the signed integer as an unsigned
  // hexadecimal number.
  llvm::SmallString<40> HexResult;
  Result.toString(HexResult, 16, /*Signed =*/false, /*Literal =*/true);

  // If we are only missing a sign bit, this is less likely to result in actual
  // bugs -- if the result is cast back to an unsigned type, it will have the
  // expected value. Thus we place this behind a different warning that can be
  // turned off separately if needed.
  if (LeftBits == ResultBits - 1) {
    S.Diag(Loc, diag::warn_shift_result_sets_sign_bit)
        << HexResult.str() << LHSTy
        << lex.get()->getSourceRange() << rex.get()->getSourceRange();
    return;
  }

  S.Diag(Loc, diag::warn_shift_result_gt_typewidth)
    << HexResult.str() << Result.getMinSignedBits() << LHSTy
    << Left.getBitWidth() << lex.get()->getSourceRange()
    << rex.get()->getSourceRange();
}
/// \brief Check the validity of a binary arithmetic operation w.r.t. pointer
/// operands.
///
/// This routine will diagnose any invalid arithmetic on pointer operands much
/// like \see checkArithmeticOpPointerOperand. However, it has special logic
/// for emitting a single diagnostic even for operations where both LHS and RHS
/// are (potentially problematic) pointers.
///
/// \returns True when the operand is valid to use (even if as an extension).
static bool checkArithmeticBinOpPointerOperands(Sema &S, SourceLocation Loc,
                                                Expr *LHS, Expr *RHS) {
  bool isLHSPointer = LHS->getType()->isAnyPointerType();
  bool isRHSPointer = RHS->getType()->isAnyPointerType();
  if (!isLHSPointer && !isRHSPointer) return true;

  QualType LHSPointeeTy, RHSPointeeTy;
  if (isLHSPointer) LHSPointeeTy = LHS->getType()->getPointeeType();
  if (isRHSPointer) RHSPointeeTy = RHS->getType()->getPointeeType();

  // Check for arithmetic on pointers to incomplete types.
  bool isLHSVoidPtr = isLHSPointer && LHSPointeeTy->isVoidType();
  bool isRHSVoidPtr = isRHSPointer && RHSPointeeTy->isVoidType();
  if (isLHSVoidPtr || isRHSVoidPtr) {
    if (!isRHSVoidPtr) diagnoseArithmeticOnVoidPointer(S, Loc, LHS);
    else if (!isLHSVoidPtr) diagnoseArithmeticOnVoidPointer(S, Loc, RHS);
    else diagnoseArithmeticOnTwoVoidPointers(S, Loc, LHS, RHS);

    return !S.getLangOptions().CPlusPlus;
  }

  bool isLHSFuncPtr = isLHSPointer && LHSPointeeTy->isFunctionType();
  bool isRHSFuncPtr = isRHSPointer && RHSPointeeTy->isFunctionType();
  if (isLHSFuncPtr || isRHSFuncPtr) {
    if (!isRHSFuncPtr) diagnoseArithmeticOnFunctionPointer(S, Loc, LHS);
    else if (!isLHSFuncPtr) diagnoseArithmeticOnFunctionPointer(S, Loc, RHS);
    else diagnoseArithmeticOnTwoFunctionPointers(S, Loc, LHS, RHS);

    return !S.getLangOptions().CPlusPlus;
  }

  Expr *Operands[] = { LHS, RHS };
  for (unsigned i = 0; i < 2; ++i) {
    Expr *Operand = Operands[i];
    if ((Operand->getType()->isPointerType() &&
         !Operand->getType()->isDependentType()) ||
        Operand->getType()->isObjCObjectPointerType()) {
      QualType PointeeTy = Operand->getType()->getPointeeType();
      if (S.RequireCompleteType(
            Loc, PointeeTy,
            S.PDiag(diag::err_typecheck_arithmetic_incomplete_type)
              << PointeeTy << Operand->getSourceRange()))
        return false;
    }
  }
  return true;
}
void Sema::CheckArrayAccess(const Expr *expr) {
  //int AllowOnePastEnd = 0;
  while (expr) {
    //expr = expr->IgnoreParenImpCasts();
    switch (expr->getStmtClass()) {
      case Stmt::ArraySubscriptExprClass: {
        const ArraySubscriptExpr *ASE = cast<ArraySubscriptExpr>(expr);
        CheckArrayAccess(ASE->getBase(), ASE->getIdx(), true);//, AllowOnePastEnd > 0);
        return;
      }
      default:
        return;
    }
  }
}
void Sematic::CheckArrayAccess(const Expr *BaseExpr, const Expr *IndexExpr,
                            bool isSubscript, bool AllowOnePastEnd) {
  const Type* EffectiveType = getElementType(BaseExpr);
  BaseExpr = BaseExpr->IgnoreParenCasts();
  IndexExpr = IndexExpr->IgnoreParenCasts();

  const ConstantArrayType *ArrayTy =
    Context.getAsConstantArrayType(BaseExpr->getType());
  if (!ArrayTy)
    return;

  //if (IndexExpr->isValueDependent())
  //  return;
  llvm::APSInt index;
  if (!IndexExpr->isIntegerConstantExpr(index, Context))
    return;

  //const NamedDecl *ND = NULL;
  //if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(BaseExpr))
  //  ND = dyn_cast<NamedDecl>(DRE->getDecl());
  //if (const MemberExpr *ME = dyn_cast<MemberExpr>(BaseExpr))
  //  ND = dyn_cast<NamedDecl>(ME->getMemberDecl());

  if (index.isUnsigned() || !index.isNegative()) {
    llvm::APInt size = ArrayTy->getSize();
    if (!size.isStrictlyPositive())
      return;

    const Type* BaseType = getElementType(BaseExpr);
    if (!isSubscript && BaseType != EffectiveType) {
      // Make sure we're comparing apples to apples when comparing index to size
      uint64_t ptrarith_typesize = Context.getTypeSize(EffectiveType);
      uint64_t array_typesize = Context.getTypeSize(BaseType);
      // Handle ptrarith_typesize being zero, such as when casting to void*
      if (!ptrarith_typesize) ptrarith_typesize = 1;
      if (ptrarith_typesize != array_typesize) {
        // There's a cast to a different size type involved
        uint64_t ratio = array_typesize / ptrarith_typesize;
        // TODO: Be smarter about handling cases where array_typesize is not a
        // multiple of ptrarith_typesize
        if (ptrarith_typesize * ratio == array_typesize)
          size *= llvm::APInt(size.getBitWidth(), ratio);
      }
    }

    if (size.getBitWidth() > index.getBitWidth())
      index = index.sext(size.getBitWidth());
    else if (size.getBitWidth() < index.getBitWidth())
      size = size.sext(index.getBitWidth());

    // For array subscripting the index must be less than size, but for pointer
    // arithmetic also allow the index (offset) to be equal to size since
    // computing the next address after the end of the array is legal and
    // commonly done e.g. in C++ iterators and range-based for loops.
    if (AllowOnePastEnd ? index.sle(size) : index.slt(size))
      return;
    unsigned DiagID = diag::warn_ptr_arith_exceeds_bounds;
    if (isSubscript)
      DiagID = diag::warn_array_index_exceeds_bounds;

    DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
                        PDiag(DiagID) << index.toString(10, true)
                          << size.toString(10, true)
                          << (unsigned)size.getLimitedValue(~0U)
                          << IndexExpr->getSourceRange());
  } else {
    unsigned DiagID = diag::warn_array_index_precedes_bounds;
    if (!isSubscript) {
      DiagID = diag::warn_ptr_arith_precedes_bounds;
      if (index.isNegative()) index = -index;
    }

    DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
                        PDiag(DiagID) << index.toString(10, true)
                          << IndexExpr->getSourceRange());
  }

  //if (ND)
  //  DiagRuntimeBehavior(ND->getLocStart(), BaseExpr,
  //                      PDiag(diag::note_array_index_out_of_bounds)
  //                        << ND->getDeclName());
}
ExprResult
Sematic::analyzeArraySubscript(Expr *Base, SourceLocation LLoc,
                              Expr *Idx, SourceLocation RLoc) {
  // Since this might be a postfix expression, get rid of ParenListExprs.
  //ExprResult Result = MaybeConvertParenListExprToParenExpr(S, Base);
  //if (Result.isInvalid()) return ExprError();
  //Base = Result.take();

  //Expr *LHSExp = Base, *RHSExp = Idx;

  //return CreateBuiltinArraySubscriptExpr(Base, LLoc, Idx, RLoc);
  Expr *LHSExp = Base;
  Expr *RHSExp = Idx;

  // Perform default conversions.
  //if (!LHSExp->getType()->getAs<VectorType>()) {
  //  ExprResult Result = DefaultFunctionArrayLvalueConversion(LHSExp);
  //  if (Result.isInvalid())
  //    return ExprError();
  //  LHSExp = Result.take();
  //}
  //ExprResult Result = DefaultFunctionArrayLvalueConversion(RHSExp);
  //if (Result.isInvalid())
  //  return ExprError();
  //RHSExp = Result.take();

  QualType LHSTy = LHSExp->getType(), RHSTy = RHSExp->getType();

  // C99 6.5.2.1p2: the expression e1[e2] is by definition precisely equivalent
  // to the expression *((e1)+(e2)). This means the array "Base" may actually be
  // in the subscript position. As a result, we need to derive the array base
  // and index from the expression types.
  Expr *BaseExpr, *IndexExpr;
  QualType ResultType;
  if (LHSTy->isArrayType()) {
    // If we see an array that wasn't promoted by
    // DefaultFunctionArrayLvalueConversion, it must be an array that
    // wasn't promoted because of the C90 rule that doesn't
    // allow promoting non-lvalue arrays.  Warn, then
    // force the promotion here.
    //Diag(LHSExp->getLocStart(), diag::ext_subscript_non_lvalue) <<
    //    LHSExp->getSourceRange();
    LHSExp = impCastExprToType(LHSExp, Context.getArrayDecayedType(LHSTy),
                               CK_ArrayToPointerDecay).take();
    //LHSTy = LHSExp->getType();

    BaseExpr = LHSExp;
    IndexExpr = RHSExp;
    //ResultType = LHSTy->getAs<PointerType>()->getPointeeType();
    ResultType = LHSExp->getType();
  } else if (RHSTy->isArrayType()) {
    // Same as previous, except for 123[f().a] case
    //Diag(RHSExp->getLocStart(), diag::ext_subscript_non_lvalue) <<
    //    RHSExp->getSourceRange();
    RHSExp = impCastExprToType(RHSExp, Context.getArrayDecayedType(RHSTy),
                              CK_ArrayToPointerDecay).take();
    //RHSTy = RHSExp->getType();
    ResultType = RHSExp->getType();

    BaseExpr = RHSExp;
    IndexExpr = LHSExp;
    //ResultType = RHSTy->getAs<PointerType>()->getPointeeType();
  } else {
    return ExprError(Diag(LLoc, diag::err_typecheck_subscript_value)
       << LHSExp->getSourceRange() << RHSExp->getSourceRange());
  }
  // C99 6.5.2.1p1
  if (!IndexExpr->getType()->isIntegerType() && !IndexExpr->isTypeDependent())
    return ExprError(Diag(LLoc, diag::err_typecheck_subscript_not_integer)
                     << IndexExpr->getSourceRange());
  return Owned(new ArraySubscriptExpr(LHSExp, RHSExp,
                                                ResultType, VK, OK, RLoc));
}

/// This is not an AltiVec-style cast, so turn the ParenListExpr into a sequence
/// of comma binary operators.
ExprResult
Sema::MaybeConvertParenListExprToParenExpr(Scope *S, Expr *expr) {
  ParenListExpr *E = dyn_cast<ParenListExpr>(expr);
  if (!E)
    return Owned(expr);

  ExprResult Result(E->getExpr(0));

  for (unsigned i = 1, e = E->getNumExprs(); i != e && !Result.isInvalid(); ++i)
    Result = analyzeBinaryOperator(S, E->getExprLoc(), tok::comma, Result.get(),
                        E->getExpr(i));

  if (Result.isInvalid()) return ExprError();
  return Owned(new (Context) ParenExpr(E->getLParenLoc(), E->getRParenLoc(), Result.get() );
}

ExprResult
Sema::CreateBuiltinArraySubscriptExpr(Expr *Base, SourceLocation LLoc,
                                     Expr *Idx, SourceLocation RLoc) {
  Expr *LHSExp = Base;
  Expr *RHSExp = Idx;

  // Perform default conversions.
  if (!LHSExp->getType()->getAs<VectorType>()) {
    ExprResult Result = DefaultFunctionArrayLvalueConversion(LHSExp);
    if (Result.isInvalid())
      return ExprError();
    LHSExp = Result.take();
  }
  ExprResult Result = DefaultFunctionArrayLvalueConversion(RHSExp);
  if (Result.isInvalid())
    return ExprError();
  RHSExp = Result.take();

  QualType LHSTy = LHSExp->getType(), RHSTy = RHSExp->getType();
  ExprObjectKind OK = OK_Ordinary;

  // C99 6.5.2.1p2: the expression e1[e2] is by definition precisely equivalent
  // to the expression *((e1)+(e2)). This means the array "Base" may actually be
  // in the subscript position. As a result, we need to derive the array base
  // and index from the expression types.
  Expr *BaseExpr, *IndexExpr;
  QualType ResultType;
  if (LHSTy->isDependentType() || RHSTy->isDependentType()) {
    BaseExpr = LHSExp;
    IndexExpr = RHSExp;
    ResultType = Context.DependentTy;
  } else if (LHSTy->isArrayType()) {
    // If we see an array that wasn't promoted by
    // DefaultFunctionArrayLvalueConversion, it must be an array that
    // wasn't promoted because of the C90 rule that doesn't
    // allow promoting non-lvalue arrays.  Warn, then
    // force the promotion here.
    Diag(LHSExp->getLocStart(), diag::ext_subscript_non_lvalue) <<
        LHSExp->getSourceRange();
    LHSExp = impCastExprToType(LHSExp, Context.getArrayDecayedType(LHSTy),
                               CK_ArrayToPointerDecay).take();
    LHSTy = LHSExp->getType();

    BaseExpr = LHSExp;
    IndexExpr = RHSExp;
    ResultType = LHSTy->getAs<PointerType>()->getPointeeType();
  } else if (RHSTy->isArrayType()) {
    // Same as previous, except for 123[f().a] case
    Diag(RHSExp->getLocStart(), diag::ext_subscript_non_lvalue) <<
        RHSExp->getSourceRange();
    RHSExp = impCastExprToType(RHSExp, Context.getArrayDecayedType(RHSTy),
                               CK_ArrayToPointerDecay).take();
    RHSTy = RHSExp->getType();

    BaseExpr = RHSExp;
    IndexExpr = LHSExp;
    ResultType = RHSTy->getAs<PointerType>()->getPointeeType();
  } else {
    return ExprError(Diag(LLoc, diag::err_typecheck_subscript_value)
       << LHSExp->getSourceRange() << RHSExp->getSourceRange());
  }
  // C99 6.5.2.1p1
  if (!IndexExpr->getType()->isIntegerType() && !IndexExpr->isTypeDependent())
    return ExprError(Diag(LLoc, diag::err_typecheck_subscript_not_integer)
                     << IndexExpr->getSourceRange());
  return Owned(new (Context) ArraySubscriptExpr(LHSExp, RHSExp,
                                                ResultType, VK, RLoc));
}
ExprResult Sematic::analyzeUnaryOp(SourceLocation OpLoc, tok::TokenKind Kind, Expr *InputExpr) {
  UnaryOperatorKind Opc = ConvertTokenKindToUnaryOpcode(Kind);
  ExprResult Input = Owned(InputExpr);
  QualType resultType;
  switch (Opc) {
  case UO_Plus:
  case UO_Minus:
    Input = unaryConversions(Input.take());
    if (Input.isInvalid()) return ExprError();
    resultType = Input.get()->getType();
    if (resultType->isArithmeticType() ) 
      break;

    return ExprError(Diag(OpLoc, diag::err_typecheck_unary_expr)
      << resultType << Input.get()->getSourceRange());

  case UO_Not: // bitwise complement
    Input = unaryConversions(Input.take());
    if (Input.isInvalid()) return ExprError();
    resultType = Input.get()->getType();
    if (resultType->hasIntegerRepresentation())
      break;
    } else {
      return ExprError(Diag(OpLoc, diag::err_typecheck_unary_expr)
        << resultType << Input.get()->getSourceRange());
    }
    break;

  case UO_LNot: // logical negation
    // Unlike +/-/~, integer promotions aren't done here (C99 6.5.3.3p5).
    //Input = DefaultFunctionArrayLvalueConversion(Input.take());
    //if (Input.isInvalid()) return ExprError();
    resultType = Input.get()->getType();
    if (!resultType->isBooleanType() ) {
      // C99 6.5.3.3p1: ok, fallthrough;
      return ExprError(Diag(OpLoc, diag::err_typecheck_unary_expr)
        << resultType << Input.get()->getSourceRange());
    }
    
    // LNot always has type int. C99 6.5.3.3p5.
    // In C++, it's bool. C++ 5.3.1p8
    //resultType = Context.getLogicalOperationType();
    break;
  }
  if (resultType.isNull() || Input.isInvalid())
    return ExprError();

  // Check for array bounds violations in the operand of the UnaryOperator,
  // except for the '*' and '&' operators that have to be handled specially
  // by CheckArrayAccess (as there are special cases like &array[arraysize]
  // that are explicitly defined as valid by the standard).
  CheckArrayAccess(Input.get());

  return Owned(new UnaryOperator(Input.take(), Opc, resultType, OpLoc) );
}
ExprResult Sematic::analyzeBuiltinCallExpr(Expr* Fn, ExprVector& args){
  unsigned NumArgs = args.size();

  // Since this might be a postfix expression, get rid of ParenListExprs.
  //ExprResult Result = MaybeConvertParenListExprToParenExpr(Fn);
  //if (Result.isInvalid()) return ExprError();
  //Fn = Result.take();
  //ExprResult Result = unaryConversions(Fn);
  //if (Result.isInvalid())
  //  return ExprError();
  //Fn = Result.take();

  FunctionDecl* FDecl = Fn->getFuncDecl();
  if (FDecl)
    return ExprError();

  unsigned BuiltinID = FDecl->getBuiltinID();
  if (!BuiltinID)
  {
    return ExprError();
  }
  
  // Make the call expr early, before semantic checks.  This guarantees cleanup
  // of arguments and function on error.
  CallExpr *TheCall = new CallExpr(Fn, args,
                                     Context.BoolTy,
                                     VK_RValue,
                                     RParenLoc);
  //FuncT = Fn->getFuncType();
  // We know the result type of the call, set it.
  TheCall->setType(FDecl->getCallResultType() );
  TheCall->setValueKind(Expr::getValueKindForType(FDecl->getResultType()));//Lvalue Rvalue
  // C99 6.5.2.2p7 - the arguments are implicitly converted, as if by
  // assignment, to the types of the corresponding parameter, ...
  unsigned NumArgsInProto = FDecl->getProtoNumArgs();
  bool Invalid = false;

  if (NumArgs != NumArgsInProto) {
    Diag();
    TheCall.release();
    return ExprError();
  }
  for (unsigned i = 0; i != NumArgs; i++) {
    Expr *Arg = Args[i];

    CheckArrayAccess(Arg);
      //InitializedEntity Entity
      //  = InitializedEntity::InitializeParameter(Context, 
      //                                           Proto->getArgType(i),
      //                                           Proto->isArgConsumed(i));
      //ExprResult ArgE = PerformCopyInitialization(Entity,
      //                                            SourceLocation(),
      //                                            Owned(Arg));
      ExprResult ArgE = parameterConversion(Owned(Arg), FDecl->getArgType(i) );
      if (ArgE.isInvalid())
        return true;
      
      Arg = ArgE.takeAs<Expr>();
    
    TheCall->setArg(i, Arg);
  }
  return TheCall.move();
  //return CheckBuiltinFunctionCall(BuiltinID, TheCall);
}
ExprResult Sematic::parameterConversion(const ExprResult& Arg, const QualType& protoType){
  ExprResult retArg = Arg.move();
  QualType argType = retArg.get()->getQualType();
  ImpliConversionKind ick = implicitCast(argType, protoType);
  if (ick == kImpliUncast){
    diag(argType not suitable for protoType);
    return ExprError();
  } else if (ick != kIdentity){
    return impCastExprToType(retArg, protoType, ick);(sema.cpp)
   }
   return Arg;
}
ExprResult Sematic::impCastExprToType(Expr *E, QualType Ty,
                                   CastKind Kind, ExprValueKind VK){

  QualType ExprTy = E->getType();
  QualType TypeTy = Ty;

  if (ExprTy == TypeTy)
    return Owned(E);
  if (ImplicitCastExpr *ImpCast = dyn_cast<ImplicitCastExpr>(E)) {
    if (ImpCast->getCastKind() == Kind) {
      ImpCast->setType(Ty);
      ImpCast->setValueKind(VK);
      return Owned(E);
    }
  }

  return Owned(ImplicitCastExpr::Create(Ty, Kind, E) );
}
ImplicitConversionKind Sematic::implicitCast(QualType& FromType, const QualType& toType){
  ImplicitConversionKind ICK = kImplicitIdentity;
  if (FromType.isIntegralPromotion(ToType) ) {
    // Integral promotion (C++ 4.5).
    ICK = kImplicitIntegralPromotion;
    FromType = ToType;
  } else if ((FromType->isRealFloatingType() &&
    ToType->isIntegralType() ) ||
    (FromType->isIntegralType() &&
    ToType->isRealFloatingType())) {
      // Floating-integral conversions (C++ 4.9).
      ICK = kImplicitFloatingIntegralCast;
      FromType = ToType;
  }else{
    ICK = kImplicitUncast;
  }
  return ICK;
}
ExprResult
Sematic::CheckBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  ExprResult TheCallResult(Owned(TheCall));

  // Find out if any arguments are required to be integer constant expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  Context.GetBuiltinType(BuiltinID, Error, &ICEArguments);
  if (Error != ASTContext::GE_None)
    ICEArguments = 0;  // Don't diagnose previously diagnosed errors.
  
  // If any arguments are required to be ICE's, check and diagnose.
  for (unsigned ArgNo = 0; ICEArguments != 0; ++ArgNo) {
    // Skip arguments not required to be ICE's.
    if ((ICEArguments & (1 << ArgNo)) == 0) continue;
    
    llvm::APSInt Result;
    if (SemaBuiltinConstantArg(TheCall, ArgNo, Result))
      return true;
    ICEArguments &= ~(1 << ArgNo);
  }
  
  return move(TheCallResult);
}
/// SemaBuiltinConstantArg - Handle a check if argument ArgNum of CallExpr
/// TheCall is a constant expression.
bool Sema::SemaBuiltinConstantArg(CallExpr *TheCall, int ArgNum,
                                  llvm::APSInt &Result) {
  Expr *Arg = TheCall->getArg(ArgNum);
  DeclRefExpr *DRE =cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());
  FunctionDecl *FDecl = cast<FunctionDecl>(DRE->getDecl());
  
  if (Arg->isTypeDependent() || Arg->isValueDependent()) return false;
  
  if (!Arg->isIntegerConstantExpr(Result, Context))
    return Diag(TheCall->getLocStart(), diag::err_constant_integer_arg_type)
                << FDecl->getDeclName() <<  Arg->getSourceRange();
  
  return false;
}
/// ActOnCallExpr - Handle a call to Fn with the specified array of arguments.
/// This provides the location of the left/right parens and a list of comma
/// locations.
ExprResult
Sema::ActOnCallExpr(Expr *Fn, SourceLocation LParenLoc,
                    /*MultiExprArg*/ExprVector& args, SourceLocation RParenLoc) {
  unsigned NumArgs = args.size();

  // Since this might be a postfix expression, get rid of ParenListExprs.
  ExprResult Result = MaybeConvertParenListExprToParenExpr(S, Fn);
  if (Result.isInvalid()) return ExprError();
  Fn = Result.take();

  Expr **Args = args.release();

  // If we're directly calling a function, get the appropriate declaration.

  Expr *NakedFn = Fn->IgnoreParens();


  return BuildResolvedCallExpr(Fn, NDecl, LParenLoc, Args, NumArgs, RParenLoc,
                               ExecConfig);
}
/// BuildResolvedCallExpr - Build a call to a resolved expression,
/// i.e. an expression not of \p OverloadTy.  The expression should
/// unary-convert to an expression of function-pointer or
/// block-pointer type.
///
/// \param NDecl the declaration being called, if available
ExprResult
Sema::BuildResolvedCallExpr(Expr *Fn, SourceLocation LParenLoc,
                            Expr **Args, unsigned NumArgs,
                            SourceLocation RParenLoc) {
  //FunctionDecl *FDecl = dyn_cast_or_null<FunctionDecl>(NDecl);

  // Promote the function operand.
  ExprResult Result = unaryConversions(Fn);
  if (Result.isInvalid())
    return ExprError();
  Fn = Result.take();

  // Make the call expr early, before semantic checks.  This guarantees cleanup
  // of arguments and function on error.
  CallExpr *TheCall = new (Context) CallExpr(Fn,
                                     Args, NumArgs,
                                     Context.BoolTy,
                                     VK_RValue,
                                     RParenLoc);
  FunctionDecl FDecl = Fn->getFuncDecl();

  unsigned BuiltinID = FDecl->getBuiltinID();

 //retry:
 // const FunctionType *FuncT;
 // if (const PointerType *PT = Fn->getType()->getAs<PointerType>()) {
 //   // C99 6.5.2.2p1 - "The expression that denotes the called function shall
 //   // have type pointer to function".
 //   FuncT = PT->getPointeeType()->getAs<FunctionType>();
 //   if (FuncT == 0)
 //     return ExprError(Diag(LParenLoc, diag::err_typecheck_call_not_function)
 //                        << Fn->getType() << Fn->getSourceRange());
 // } else if (const BlockPointerType *BPT =
 //              Fn->getType()->getAs<BlockPointerType>()) {
 //   FuncT = BPT->getPointeeType()->castAs<FunctionType>();
 // } else {
 //   // Handle calls to expressions of unknown-any type.
 //   if (Fn->getType() == Context.UnknownAnyTy) {
 //     ExprResult rewrite = rebuildUnknownAnyFunction(*this, Fn);
 //     if (rewrite.isInvalid()) return ExprError();
 //     Fn = rewrite.take();
 //     TheCall->setCallee(Fn);
 //     goto retry;
 //   }

 //   return ExprError(Diag(LParenLoc, diag::err_typecheck_call_not_function)
 //     << Fn->getType() << Fn->getSourceRange());
 // }

  FuncT = Fn->getFuncType();
  // We know the result type of the call, set it.
  TheCall->setType(FuncT->getCallResultType() );
  TheCall->setValueKind(Expr::getValueKindForType(FuncT->getResultType()));//Lvalue Rvalue
  if (ConvertArgumentsForCall(TheCall, Fn, FDecl, Proto, Args, NumArgs,
                              RParenLoc))
    return ExprError();

  //if (const FunctionProtoType *Proto = dyn_cast<FunctionProtoType>(FuncT)) {
  //  if (ConvertArgumentsForCall(TheCall, Fn, FDecl, Proto, Args, NumArgs,
  //                              RParenLoc))
  //    return ExprError();
  //} else {
  //  assert(isa<FunctionNoProtoType>(FuncT) && "Unknown FunctionType!");

  //  if (FDecl) {
  //    // Check if we have too few/too many template arguments, based
  //    // on our knowledge of the function definition.
  //    const FunctionDecl *Def = 0;
  //    if (FDecl->hasBody(Def) && NumArgs != Def->param_size()) {
  //      const FunctionProtoType *Proto 
  //        = Def->getType()->getAs<FunctionProtoType>();
  //      if (!Proto || !(Proto->isVariadic() && NumArgs >= Def->param_size()))
  //        Diag(RParenLoc, diag::warn_call_wrong_number_of_arguments)
  //          << (NumArgs > Def->param_size()) << FDecl << Fn->getSourceRange();
  //    }
  //    
  //    // If the function we're calling isn't a function prototype, but we have
  //    // a function prototype from a prior declaratiom, use that prototype.
  //    if (!FDecl->hasPrototype())
  //      Proto = FDecl->getType()->getAs<FunctionProtoType>();
  //  }

  //}

  // Check for sentinels
  //if (NDecl)
  //  DiagnoseSentinelCalls(NDecl, LParenLoc, Args, NumArgs);

  // Do special checking on direct calls to functions.
  //if (FDecl) {
  //  if (CheckFunctionCall(FDecl, TheCall))
  //    return ExprError();

    if (BuiltinID)
      return CheckBuiltinFunctionCall(BuiltinID, TheCall);
    else
      return ExprError();
}
bool
Sema::ConvertArgumentsForCall(CallExpr *Call, Expr *Fn,
                              FunctionDecl *FDecl,
                              const FunctionProtoType *Proto,
                              Expr **Args, unsigned NumArgs,
                              SourceLocation RParenLoc) {
  // C99 6.5.2.2p7 - the arguments are implicitly converted, as if by
  // assignment, to the types of the corresponding parameter, ...
  unsigned NumArgsInProto = Proto->getNumArgs();
  bool Invalid = false;

  if (NumArgs != NumArgsInProto) {
    Diag();
    return true;
  }
  // If too few arguments are available (and we don't have default
  // arguments for the remaining parameters), don't make the call.
  //if (NumArgs < NumArgsInProto) {
  //  if (!FDecl || NumArgs < FDecl->getMinRequiredArguments()) {
  //    Diag(RParenLoc, diag::err_typecheck_call_too_few_args)
  //      << Fn->getType()->isBlockPointerType()
  //      << NumArgsInProto << NumArgs << Fn->getSourceRange();

  //    // Emit the location of the prototype.
  //    if (FDecl && !FDecl->getBuiltinID())
  //      Diag(FDecl->getLocStart(), diag::note_callee_decl)
  //        << FDecl;

  //    return true;
  //  }
  //  Call->setNumArgs(Context, NumArgsInProto);
  //}

  // If too many are passed and not variadic, error on the extras and drop
  // them.
  //if (NumArgs > NumArgsInProto) {
  //  if (!Proto->isVariadic()) {
  //    Diag(Args[NumArgsInProto]->getLocStart(),
  //         diag::err_typecheck_call_too_many_args)
  //      << Fn->getType()->isBlockPointerType()
  //      << NumArgsInProto << NumArgs << Fn->getSourceRange()
  //      << SourceRange(Args[NumArgsInProto]->getLocStart(),
  //                     Args[NumArgs-1]->getLocEnd());

  //    // Emit the location of the prototype.
  //    if (FDecl && !FDecl->getBuiltinID())
  //      Diag(FDecl->getLocStart(), diag::note_callee_decl)
  //        << FDecl;
  //    
  //    // This deletes the extra arguments.
  //    Call->setNumArgs(Context, NumArgsInProto);
  //    return true;
  //  }
  //}
  //SmallVector<Expr *, 8> AllArgs;
  //Invalid = GatherArgumentsForCall(Call->getSourceRange().getBegin(), FDecl,
  //                                 Proto, 0, Args, NumArgs, AllArgs);
    // Promote the arguments (C99 6.5.2.2p6).
    for (unsigned i = 0; i != NumArgs; i++) {
      Expr *Arg = Args[i];

      if (FDecl && i < FDecl->getNumArgs()) {
        //InitializedEntity Entity
        //  = InitializedEntity::InitializeParameter(Context, 
        //                                           Proto->getArgType(i),
        //                                           Proto->isArgConsumed(i));
        //ExprResult ArgE = PerformCopyInitialization(Entity,
        //                                            SourceLocation(),
        //                                            Owned(Arg));
        ExprResult ArgE = paramterConversion(Owned(Arg) );
        if (ArgE.isInvalid())
          return true;
        
        Arg = ArgE.takeAs<Expr>();
      }
      
      if (RequireCompleteType(Arg->getSourceRange().getBegin(),
                              Arg->getType(),
                              PDiag(diag::err_call_incomplete_argument)
                                << Arg->getSourceRange()))
        return ExprError();

      CheckArrayAccess(Arg);
      TheCall->setArg(i, Arg);
    }
  //if (Invalid)
  //  return true;
  //unsigned TotalNumArgs = AllArgs.size();
  //for (unsigned i = 0; i < TotalNumArgs; ++i)
  //  Call->setArg(i, AllArgs[i]);

  return false;
}
bool Sema::GatherArgumentsForCall(SourceLocation CallLoc,
                                  FunctionDecl *FDecl,
                                  const FunctionProtoType *Proto,
                                  unsigned FirstProtoArg,
                                  Expr **Args, unsigned NumArgs,
                                  SmallVector<Expr *, 8> &AllArgs) {
  unsigned NumArgsInProto = Proto->getNumArgs();
  unsigned NumArgsToCheck = NumArgs;
  bool Invalid = false;
  if (NumArgs != NumArgsInProto)
    // Use default arguments for missing arguments
    NumArgsToCheck = NumArgsInProto;
  unsigned ArgIx = 0;
  // Continue to check argument types (even if we have too few/many args).
  for (unsigned i = FirstProtoArg; i != NumArgsToCheck; i++) {
    QualType ProtoArgType = Proto->getArgType(i);

    Expr *Arg;
    if (ArgIx < NumArgs) {
      Arg = Args[ArgIx++];

      if (RequireCompleteType(Arg->getSourceRange().getBegin(),
                              ProtoArgType,
                              PDiag(diag::err_call_incomplete_argument)
                              << Arg->getSourceRange()))
        return true;

      // Pass the argument
      ParmVarDecl *Param = 0;
      if (FDecl && i < FDecl->getNumParams())
        Param = FDecl->getParamDecl(i);

      InitializedEntity Entity =
        Param? InitializedEntity::InitializeParameter(Context, Param)
             : InitializedEntity::InitializeParameter(Context, ProtoArgType,
                                                      Proto->isArgConsumed(i));
      ExprResult ArgE = PerformCopyInitialization(Entity,
                                                  SourceLocation(),
                                                  Owned(Arg));
      if (ArgE.isInvalid())
        return true;

      Arg = ArgE.takeAs<Expr>();
    } else {
      ParmVarDecl *Param = FDecl->getParamDecl(i);

      ExprResult ArgExpr =
        BuildCXXDefaultArgExpr(CallLoc, FDecl, Param);
      if (ArgExpr.isInvalid())
        return true;

      Arg = ArgExpr.takeAs<Expr>();
    }

    // Check for array bounds violations for each argument to the call. This
    // check only triggers warnings when the argument isn't a more complex Expr
    // with its own checking, such as a BinaryOperator.
    CheckArrayAccess(Arg);

    AllArgs.push_back(Arg);
  }
}
}
}
