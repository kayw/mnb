namespace mnb{
namespace expr{
bool QualType::isIntegralPromotion(const QualType& toType){
  // An rvalue of type char, signed char, unsigned char, short int, or
  // unsigned short int can be converted to an rvalue of type int if
  // int can represent all the values of the source type; otherwise,
  // the source rvalue can be converted to an rvalue of type unsigned
  // int (C++ 4.5p1).
  if (pType_->isIntegerType() && toType.get()->isIntegerType() )
    if (// We can promote any signed, promotable integer type to an int
        //(isSignedIntegerType() ||
         // We can promote any unsigned integer type whose size is
         // less than int to an int.
         (//!isSignedIntegerType() &&
          pType_->getTypeWidth() < toType.get()->getTypeWidth())) {
      return true;
    }
}
std::map<OperatorKind, visitBinOp> BinaryOperatorExpr::BinOpFunctors_ = BinaryOperatorExpr::initializeBinOpMap();
std::map<OperatorKind, visitBinOp> BinaryOperatorExpr::initializeBinOpMap(){
  std::map<OperatorKind, visitBinOp> BinOpFunctors;
  BinOpFunctors[kBOMul] = &visitMulOpExpr;
  BinOpFunctors[kBOAdd] = &visitAddOpExpr;
  BinOpFunctors[kBOSub] = &visitSubOpExpr;
  BinOpFunctors[kBODiv] = &visitDivOpExpr;
  BinOpFunctors[kBORem] = &visitRemOpExpr;
  BinOpFunctors[kBOShl] = &visitShlOpExpr;
  BinOpFunctors[kBOShr] = &visitShrOpExpr;
  BinOpFunctors[kBOAssign] = &visitAssignOpExpr;
  BinOpFunctors[kBO_LE] = &visitLEOpExpr;
  BinOpFunctors[kBO_LT] = &visitLTOpExpr;
  BinOpFunctors[kBO_GE] = &visitGEOpExpr;
  BinOpFunctors[kBO_GT] = &visitGTOpExpr;
  BinOpFunctors[kBO_EQ] = &visitEQOpExpr;
  BinOpFunctors[kBO_NE] = &visitNEOpExpr;
  BinOpFunctors[kBOBitAnd] = &visitBitwiseAndOpExpr;
  BinOpFunctors[kBOBitXor] = &visitBitwiseXorOpExpr;
  BinOpFunctors[kBOBitOr] = &visitBitwiseOrOpExpr;
  BinOpFunctors[kBOLAnd] = &visitLogicalAndOpExpr;
  BinOpFunctors[kBOLOr] = &visitLogicalOrOpExpr;
  return BinOpFunctors;
}
ExprValue BinaryOperatorExpr::success(const ExprValue& resultVal){
  assert(resultVal.valueType == ResultTy_.get()->getTypeKind() && "invalid evaluation result");
  return resultVal;
  //resultVal.setSign(resultTy.IsUnsigned() );
}

BinaryOperatorExpr::BinaryOperatorExpr(ExprResult lhs, ExprResult rhs, OperatorKind opk, QualType resultTy)
  :Lhs_(lhs)
  ,Rhs_(rhs)
  ,Opk_(opk)
  ,ResultTy_(resultTy){
}
ExprValue BinaryOperatorExpr::evaluate(){
  assert(BinOpFunctors_.find(Opk_) != BinOpFunctors_.end() );
  return BinOpFunctors_[Opk_]();
}

ExprValue BinaryOperatorExpr::visitMulOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal*rhsVal);
}

ExprValue BinaryOperatorExpr::visitAddOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal + rhsVal);
}

ExprValue BinaryOperatorExpr::visitSubOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal - rhsVal);
}

ExprValue BinaryOperatorExpr::visitDivOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal / rhsVal);
}

ExprValue BinaryOperatorExpr::visitShlOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal << rhsVal);
}
ExprValue BinaryOperatorExpr::visitShrOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal >> rhsVal);
}
ExprValue BinaryOperatorExpr::visitAssignOpExpr(){
  ExprValue rhsVal = Rhs_.evaluate();
  return rhsVal;
}
ExprValue BinaryOperatorExpr::visitLEOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal <= rhsVal);
}
ExprValue BinaryOperatorExpr::visitLTOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal < rhsVal);
}
ExprValue BinaryOperatorExpr::visitGEOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal >= rhsVal);
}
ExprValue BinaryOperatorExpr::visitGTOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal > rhsVal);
}
ExprValue BinaryOperatorExpr::visitEQOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal == rhsVal);
}
ExprValue BinaryOperatorExpr::visitNEOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal != rhsVal);
}
ExprValue BinaryOperatorExpr::visitBitwiseAndOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal & rhsVal);
}

ExprValue BinaryOperatorExpr::visitBitwiseXorOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal ^ rhsVal);
}
ExprValue BinaryOperatorExpr::visitBitwiseOrOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal | rhsVal);
}
ExprValue BinaryOperatorExpr::visitLogicalAndOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  if (!lhsVal.isTrue() )
    return success(lhsVal);
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal && rhsVal);
}
ExprValue BinaryOperatorExpr::visitLogicalOrOpExpr(){
  ExprValue lhsVal = Lhs_.evaluate();
  if (lhsVal.isTrue() )
    return success(lhsVal);
  ExprValue rhsVal = Rhs_.evaluate();
  return success(lhsVal || rhsVal);
}

ExprValue CastExpr::evaluate(){
  ExprValue subValue = OpExpr_.get()->evaluate();
  switch(CKind_){
    case kCastIntegral:
      //assert(ResultTy_.isIntegerType() );
      return success(handleIntToIntCast(subValue) );
    case kCastIntegralToBoolean:
    case kCastFloatingToBoolean:
      return success(handleConversionToBool(subValue) );
    case kCastFloatingToIntegral:
      return success(handleFloatToIntCast(subValue) );
    case kCastIntegralToFloating:
      return success(handleIntToFloatCast(subValue) );
    default:
      return success(subValue);
  }
}

ExprValue CastExpr::handleIntToIntCast(const ExprValue& subValue) {
  assert(subValue.valueType == kIntTy);
  ExprValue result;
  result.valueType = kIntTy;
  result.intVal.uintValue = subValue.intVal.uintValue;
  result.truncToWidth(ResultTy_.getTypeWidth() );
  result.intVal.isSigned = ResultTy_.isSignedIntegerType();
  return result;
}

ExprValue CastExpr::handleConversionToBool(const ExprValue& subValue) {
  ExprValue result;
  result.valueType = kBoolTy;
  assert((subValue.valueType != kBoolTy) && "You needn't cast BOOL to BOOL");
  if (subValue.valueType == kIntTy)
    result.boolVal = subValue.intVal.uintValue == 0;
  else
    result.boolVal = fabs(subValue.floatVal) < std::numeric_limits::epislon;
  return result;
}

ExprValue CastExpr::handleFloatToIntCast(const ExprValue& subValue) {
  assert(subValue.valueType == kFloatTy);
  ExprValue result;
  result.valueType = kIntTy;
  result.intVal.uintValue = static_cast<uint32_t>(subValue.floatVal);
  result.intVal.isSigned = ResultTy_.isSignedIntegerType();
  return result;
}


ExprValue CastExpr::handleIntToFloatCast(const ExprValue& subValue) {
  assert(subValue.valueType == kIntTy);
  ExprValue result;
  result.valueType = kFloatTy;
  if (subValue.isNegative() )
    result.floatVal = static_cast<float>(-subValue.intVal.uintValue);
  else
    result.floatVal = static_cast<float>(subValue.intVal.uintValue);
  return result;
}

ExprValue BuiltinCallExpr::evaluate(){
  return (*FDecl)()(arguE);
}

ExprValue UnaryOperatorExpr::evaluate(){
  ExprValue res = Input_->evaluate();
  switch(Opc_){
    case kUOPlus:
      return res;    
    case kUOMinus:
      return -res;

    case kUOLNot:
      return !res;
    default:
      assert(false);
  }
}

ExprValue ArraySubscriptExpr::evaluate() {
  std::vector<ExprValue>& evv = baseExpr_->getInitValueList();
  ExprValue ev = indexExpr_.evaluate();
  return success(evv[ev.intVal.uintValue], resultTy_);
}

ExprValue VarDecl::evaluate() {
  return success(VarValue_.evaluate(), resultTy_);
}

}
}
