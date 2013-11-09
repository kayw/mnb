#include "ast.h"

#include <sstream>
#include "builtinfn.h"
#include "symbols.h"

namespace mnb{
namespace expr{

bool QualType::isIntegralPromotion(const QualType& toType) {
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
  return false;
}

ExprResult QualType::createDefaultInit(IdentifierTable& table) {
  switch(pType_->getKind() ) 
  {
  case kBoolTy:
    return ExprResult(new Constant(false,QualType(table.typeOf(Token::kw_BOOL) ) ) );
  case kFloatTy:
    return ExprResult(new Constant(float(0.0),QualType(table.typeOf(Token::kw_REAL) ) ) );
  case kIntTy:
    return ExprResult(new Constant(uint32_t(0U), QualType(table.typeOf(Token::kw_INT) ) ) );
  default:
    return ExprResult(true);
  }
}

void QualType::setArraySize(const int32_t newSize) {
  assert(isArrayElement());
  ConstArrayType* pArrayType = static_cast<ConstArrayType*>(pType_);
  pArrayType->setArraySize(newSize); 
}
MString ConstArrayType::getTypeLiteral() const { // FIXME: mstring format
  std::ostringstream ss;
  ss << arraySize_ << " dimensional of array " << elemBase_.get()->getTypeLiteral();
  return ss.str();
}

std::map<OperatorKind, BinaryOperatorExpr::visitBinOp> BinaryOperatorExpr::binOpFunctors_ = BinaryOperatorExpr::initializeBinOpMap();
std::map<OperatorKind, BinaryOperatorExpr::visitBinOp> BinaryOperatorExpr::initializeBinOpMap(){
  std::map<OperatorKind, BinaryOperatorExpr::visitBinOp> BinOpFunctors;
  BinOpFunctors[kBOMul]     = &BinaryOperatorExpr::visitMulOpExpr;
  BinOpFunctors[kBOAdd]     = &BinaryOperatorExpr::visitAddOpExpr;
  BinOpFunctors[kBOSub]     = &BinaryOperatorExpr::visitSubOpExpr;
  BinOpFunctors[kBODiv]     = &BinaryOperatorExpr::visitDivOpExpr;
  BinOpFunctors[kBORem]     = &BinaryOperatorExpr::visitRemOpExpr;
  BinOpFunctors[kBOShl]     = &BinaryOperatorExpr::visitShlOpExpr;
  BinOpFunctors[kBOShr]     = &BinaryOperatorExpr::visitShrOpExpr;
  BinOpFunctors[kBOAssign]  = &BinaryOperatorExpr::visitAssignOpExpr;
  BinOpFunctors[kBOLE]      = &BinaryOperatorExpr::visitLEOpExpr;
  BinOpFunctors[kBOLT]      = &BinaryOperatorExpr::visitLTOpExpr;
  BinOpFunctors[kBOGE]      = &BinaryOperatorExpr::visitGEOpExpr; 
  BinOpFunctors[kBOGT]      = &BinaryOperatorExpr::visitGTOpExpr;
  BinOpFunctors[kBOEQ]      = &BinaryOperatorExpr::visitEQOpExpr;
  BinOpFunctors[kBONE]      = &BinaryOperatorExpr::visitNEOpExpr;
  BinOpFunctors[kBOBitAnd]  = &BinaryOperatorExpr::visitBitwiseAndOpExpr;
  BinOpFunctors[kBOBitXor]  = &BinaryOperatorExpr::visitBitwiseXorOpExpr;
  BinOpFunctors[kBOBitOr]   = &BinaryOperatorExpr::visitBitwiseOrOpExpr;
  BinOpFunctors[kBOLAnd]    = &BinaryOperatorExpr::visitLogicalAndOpExpr;
  BinOpFunctors[kBOLOr]     = &BinaryOperatorExpr::visitLogicalOrOpExpr;
  return BinOpFunctors;
}

void ExprNode::success(ExprValue& resultVal){
  assert(resultVal.valueType == resultTy_.get()->getKind() && "invalid evaluation result");
  if (resultVal.valueType == kIntTy)
    resultVal.intVal.isSigned = resultTy_.get()->isSigned();
  //return resultVal;
  //resultVal.setSign(resultTy.IsUnsigned() );
}

ExprValue BinaryOperatorExpr::evaluate(){
  assert(binOpFunctors_.find(opk_) != binOpFunctors_.end() );
  return (this->*binOpFunctors_[opk_])();
}

ExprValue BinaryOperatorExpr::visitMulOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.multiply(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitAddOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.add(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitSubOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.subtract(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitDivOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.div(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitRemOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  const ExprValue& rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.rem(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitShlOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.shl(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitShrOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.shr(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitAssignOpExpr(){
  ExprValue rhsVal = rhs_.evaluate();
  success(rhsVal);
  return rhsVal;
}
ExprValue BinaryOperatorExpr::visitLEOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.le(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitLTOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.lt(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitGEOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.ge(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitGTOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.gt(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitEQOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.equal(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitNEOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.neq(rhsVal);
  success(lhsVal);
  return lhsVal;
}
ExprValue BinaryOperatorExpr::visitBitwiseAndOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.bitAnd(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitBitwiseXorOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.bitXor(rhsVal);
  success(lhsVal);
  return lhsVal; 
}

ExprValue BinaryOperatorExpr::visitBitwiseOrOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  ExprValue rhsVal = rhs_.evaluate();
  lhsVal = lhsVal.bitOr(rhsVal);
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitLogicalAndOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  if (lhsVal.isTrue() ) {
    ExprValue rhsVal = rhs_.evaluate();
    lhsVal = lhsVal.logicalAnd(rhsVal);
  }
  success(lhsVal);
  return lhsVal;
}

ExprValue BinaryOperatorExpr::visitLogicalOrOpExpr(){
  ExprValue lhsVal = lhs_.evaluate();
  if (!lhsVal.isTrue() ) {
    ExprValue rhsVal = rhs_.evaluate();
    lhsVal = lhsVal.logicalOr(rhsVal);
  }
  success(lhsVal);
  return lhsVal;
}

ExprValue CastExpr::evaluate(){
  ExprValue subValue = OpExpr_.get()->evaluate();
  switch(CKind_){
    case kCastIntegral:
      //assert(getQualType().isIntegerType() );
      subValue = handleIntToIntCast(subValue);
      success(subValue);
      break;
    case kCastIntegralToBoolean:
    case kCastFloatingToBoolean:
      subValue = handleConversionToBool(subValue);
      success(subValue);
      break;
    case kCastFloatingToIntegral:
      subValue = handleFloatToIntCast(subValue);
      success(subValue);
      break; 
    case kCastIntegralToFloating:
      subValue = handleIntToFloatCast(subValue);
      success(subValue);
      break; 
    default:
      success(subValue);
      break;
  }
  return subValue;
}

ExprValue CastExpr::handleIntToIntCast(const ExprValue& subValue) {
  assert(subValue.valueType == kIntTy);
  ExprValue result;
  result.valueType = kIntTy;
  result.intVal.uintValue = subValue.intVal.uintValue;
  result.truncToWidth(getQualType().getTypeWidth() );
  result.intVal.isSigned = getQualType().isSignedInteger();
  return result;
}

ExprValue CastExpr::handleConversionToBool(const ExprValue& subValue) {
  ExprValue result;
  result.valueType = kBoolTy;
  assert((subValue.valueType != kBoolTy) && "You needn't cast BOOL to BOOL");
  if (subValue.valueType == kIntTy)
    result.boolVal = subValue.intVal.uintValue == 0;
  else
    result.boolVal = fabs(subValue.floatVal) < std::numeric_limits<float>::epsilon();
  return result;
}

ExprValue CastExpr::handleFloatToIntCast(const ExprValue& subValue) {
  assert(subValue.valueType == kFloatTy);
  ExprValue result;
  result.valueType = kIntTy;
  result.intVal.uintValue = static_cast<uint32_t>(subValue.floatVal);
  result.intVal.isSigned = getQualType().isSignedInteger();
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
  return (*pFnDecl_)(arguments_);
}

ExprValue UnaryOperatorExpr::evaluate(){
  ExprValue res = Input_->evaluate();
  switch(Opc_){
    case kUOPlus:
      return res;    
    case kUOMinus:
      return res.makeNegative();

    case kUOLNot:
      return res.makeLogicalNot();
    default:
      assert(false);
  }
}

ExprValue ArraySubscriptExpr::evaluate() {
  const std::vector<ExprValue>& evv = baseExpr_->getInitValueList();
  ExprValue ev = indexExpr_->evaluate();
  ev = evv[ev.intVal.uintValue];
  success(ev);
  return ev;
}

ExprValue VarDecl::evaluate() {
  ExprValue ev = pVarValue_->evaluate();
  success(ev);
  return ev;
}

}
}
