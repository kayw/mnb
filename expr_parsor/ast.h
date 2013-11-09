#ifndef AST_H_199850DF_581D_4723_BCBF_33E06EE91387
#define AST_H_199850DF_581D_4723_BCBF_33E06EE91387

#include <math.h>
#include <map>
#include <vector>
#include <limits>
#include "mmacros.h"
#include "mstring.h"
namespace mnb{
namespace expr{
#define MAX_BITS 32
enum TyKinds{
  kVoidTy,  kBoolTy,
  kIntTy,   kFloatTy,
};
class IdentifierTable;

class ExprValue {//todo move to cpp
  public:
    ExprValue()
      :evaluatable(false)
      ,valueType(kVoidTy){}
    explicit ExprValue(bool isSigned, uint32_t v)
      :evaluatable(false), valueType(kIntTy) {
        intVal.isSigned= isSigned;
        intVal.uintValue = v;
      } 
    explicit ExprValue(float f)
      :evaluatable(false), valueType(kFloatTy), floatVal(f) {} 
    explicit ExprValue(bool b)
      :evaluatable(false), valueType(kBoolTy), boolVal(b) {}

    bool isEqual(const ExprValue& rhs) const {
      if (valueType != rhs.valueType)
      {
        return false;
      }
      switch (valueType) {
        case kIntTy:
          return ((intVal.isSigned == rhs.intVal.isSigned) && (intVal.uintValue == rhs.intVal.uintValue));
        case kBoolTy:
          return boolVal == rhs.boolVal;
        case kFloatTy:
          return fabs(floatVal - rhs.floatVal) <= std::numeric_limits<float>::epsilon();
        default:
          return true;
      }
    }
    bool isZero() const{
      if (valueType == kIntTy) {
        return intVal.uintValue == 0;
      }
      //else if (valueType == kFloatTy) {
      //  return floatVal <= std::numeric_limits<>;
      //}
      else {
        return false;
      }
    }
    bool isNegative() const {
      if (valueType == kIntTy) {
        return intVal.isSigned && (static_cast<int32_t>(intVal.uintValue) < 0);
      //} else if (valueType == kFloatTy) {
      //  return floatVal < 0;
      } else{
        assert(false && "Value type must be INT");
      }
    }
    int32_t getValidBits() const{
      if(valueType != kIntTy)
        return 0;
      uint32_t tmp = 0;
      for (int i = MAX_BITS; i >= 0 ; --i) {
        tmp = intVal.uintValue&(~uint32_t(0UL) >> (MAX_BITS - i));
        if(tmp == 1)
          return i;
      }
      return 0;
    }
      
    bool isTrue() const {//todo
        return true;
    }
    void truncToWidth(int width) {
      if (valueType != kIntTy)
        return;
      if (width > MAX_BITS)
        width = MAX_BITS;
      intVal.uintValue &= ~uint32_t(0UL) >> (MAX_BITS - width);
    }

    //UnaryOperator
    ExprValue makeNegative() const{
      assert((valueType != kBoolTy) && "unary minus operator is not suitable for BOOL" );
      ExprValue ret;
      if (valueType == kFloatTy){
        ret.valueType = kFloatTy;
        ret.floatVal = -floatVal;
      }
      else if(valueType == kIntTy){
        ret.valueType = kIntTy;
        ret.intVal.isSigned = true;
        ret.intVal.uintValue = -intVal.uintValue;
      }
      return ret;
    }
    ExprValue makeLogicalNot() const{
      assert((valueType == kBoolTy) && "logical Not operator is only for BOOL" );
      ExprValue ret;
      ret.valueType = kBoolTy;
      ret.boolVal = !boolVal;
      return ret;
    }
    ExprValue multiply(const ExprValue& Rhs) const { //unsigned cast signed
      assert((valueType == Rhs.valueType ) && "value type mismatch!" );
      assert((valueType != kBoolTy) && "multiplication operator is not suitable for BOOL" );
      ExprValue ev;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.valueType = kFloatTy;
        ev.floatVal = floatVal * Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = floatVal * Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = intVal * Rhs.floatVal;
      } else {
        ev.valueType = kIntTy;
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signed mismatch");
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.uintValue * Rhs.intVal.uintValue;
      }
      return ev;
    }

    ExprValue add(const ExprValue& Rhs) const {
      assert((valueType == Rhs.valueType ) && "value type mismatch!" );
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "addition operator is not suitable for BOOL" );
      ExprValue ev;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.valueType = kFloatTy;
        ev.floatVal = floatVal + Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = floatVal + Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = intVal + Rhs.floatVal;
      } else {
        ev.valueType = kIntTy;
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.uintValue + Rhs.intVal.uintValue;
      }
      return ev;
    }

    ExprValue subtract(const ExprValue& Rhs) const {
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "subtraction operator is not suitable for bool");
      ExprValue ev;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.valueType = kFloatTy;
        ev.floatVal = floatVal - Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = floatVal - Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = intVal - Rhs.floatVal;
      } else {
        ev.valueType = kIntTy;
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.uintValue - Rhs.intVal.uintValue;
      }
      return ev;
    }
    ExprValue div(const ExprValue& Rhs) const {
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "divide operator is not suitable for bool" );
      ExprValue ev;
      if (valueType == kFloatTy){
        ev.valueType = kFloatTy;
        ev.floatVal = floatVal / Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = floatVal / Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.valueType = kFloatTy;
      //  ev.floatVal = intVal / Rhs.floatVal;
      } else {
        ev.valueType = kIntTy;
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.isSigned ? sDiv(Rhs) : uDiv(Rhs);//intVal / Rhs.intVal;
      }
      return ev;
    }
    uint32_t sDiv(const ExprValue& Rhs) const {
      assert((Rhs.intVal.uintValue != 0) && "divide by zero?");
      return uint32_t(static_cast<int32_t>(intVal.uintValue) / static_cast<int32_t>(Rhs.intVal.uintValue) );
    }
    uint32_t uDiv(const ExprValue& Rhs) const {
      assert((Rhs.intVal.uintValue != 0) && "divide by zero?");
      return intVal.uintValue / Rhs.intVal.uintValue;
    }

    ExprValue rem(const ExprValue& rhs) const {
      assert((valueType == kIntTy) && (rhs.valueType == kIntTy) && "remainder operator is not suitable for bool" );
      ExprValue ev;
      ev.valueType = kIntTy;
      assert((intVal.isSigned == rhs.intVal.isSigned) && "signedness mismatch");
      ev.intVal.isSigned = intVal.isSigned;
      ev.intVal.uintValue = intVal.isSigned ? sRem(rhs) : uRem(rhs);
      return ev;
    }

    uint32_t sRem(const ExprValue& Rhs) const {
      assert((Rhs.intVal.uintValue != 0) && "remainder by zero?");
      return uint32_t(static_cast<int32_t>(intVal.uintValue)%static_cast<int32_t>(Rhs.intVal.uintValue) );
    }

    uint32_t uRem(const ExprValue& Rhs) const {
      assert((Rhs.intVal.uintValue != 0) && "remainder by zero?");
      return intVal.uintValue % Rhs.intVal.uintValue;
    }

    ExprValue shr(const ExprValue& Rhs) const{
      assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "right shift operator is only suitable for INT" );
      assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
      assert((!isNegative() && !Rhs.isNegative() ) && "right shift operands must be greater than or equal to 0");
      ExprValue ev;
      ev.valueType = kIntTy;
      uint32_t shiftBits = Rhs.intVal.uintValue;
      if (shiftBits >= MAX_BITS){
        ev.intVal.uintValue = 0xFFFFFFFF;
      } else {
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.uintValue >> shiftBits;
      }
      return ev;
    }

    ExprValue shl(const ExprValue& Rhs) const{
      assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "left shift operator is only suitable for INT" );
      assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
      assert((!isNegative() && !Rhs.isNegative() ) && "left shift operands must be greater than or equal to 0");
      ExprValue ev;
      ev.valueType = kIntTy;
      uint32_t shiftBits = Rhs.intVal.uintValue;
      if (shiftBits >= MAX_BITS) {
        ev.intVal.uintValue = 0;
      } else {
        ev.intVal.isSigned = intVal.isSigned;
        ev.intVal.uintValue = intVal.uintValue << shiftBits;
      }
      return ev;
    }

    ExprValue gt(const ExprValue& Rhs) const{
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "greater than operator is not suitable for BOOL" );
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.boolVal = floatVal > Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.boolVal = floatVal > Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.boolVal = intVal > Rhs.floatVal;
      } else {
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.isSigned ? sGt(Rhs) : uGt(Rhs);
      }
      return ev;
    }
    bool sGt(const ExprValue& Rhs) const {
      return static_cast<int32_t>(intVal.uintValue) > static_cast<int32_t>(Rhs.intVal.uintValue);
    }
    bool uGt(const ExprValue& Rhs) const {
      return intVal.uintValue > Rhs.intVal.uintValue;
    }

    ExprValue ge(const ExprValue& Rhs) const{
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "greater than or equal to operator is not suitable for BOOL" );
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.boolVal = (floatVal > Rhs.floatVal) || fabs(floatVal - Rhs.floatVal) < std::numeric_limits<float>::epsilon();
      //} else if (valueType == kFloatTy){
      //  ev.boolVal = floatVal >= Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.boolVal = intVal >= Rhs.floatVal;
      } else {
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.isSigned ? sGe(Rhs) : uGe(Rhs);
      }
      return ev;
    }
    bool sGe(const ExprValue& Rhs) const {
      return static_cast<int32_t>(intVal.uintValue) >= static_cast<int32_t>(Rhs.intVal.uintValue);
    }
    bool uGe(const ExprValue& Rhs) const {
      return intVal.uintValue >= Rhs.intVal.uintValue;
    }

    ExprValue lt(const ExprValue& Rhs) const{
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "less than operator is not suitable for BOOL" );
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.boolVal = floatVal < Rhs.floatVal;
      //} else if (valueType == kFloatTy){
      //  ev.boolVal = floatVal < Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.boolVal = intVal < Rhs.floatVal;
      } else {
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.isSigned ? sLt(Rhs) : uLt(Rhs);
      }
      return ev;
    }
    bool sLt(const ExprValue& Rhs) const {
      return static_cast<int32_t>(intVal.uintValue) < static_cast<int32_t>(Rhs.intVal.uintValue);
    }
    bool uLt(const ExprValue& Rhs) const {
      return intVal.uintValue < Rhs.intVal.uintValue;
    }

    ExprValue le(const ExprValue& Rhs) const{
      assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "less than or equal to operator is not suitable for BOOL" );
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
        ev.boolVal = floatVal < Rhs.floatVal || fabs(floatVal - Rhs.floatVal) <= std::numeric_limits<float>::epsilon();
      //} else if (valueType == kFloatTy){
      //  ev.boolVal = floatVal <= Rhs.intVal;
      //} else if (Rhs.valueType == kFloatTy){
      //  ev.boolVal = intVal <= Rhs.floatVal;
      } else {
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.isSigned ? sLe(Rhs) : uLe(Rhs);
      }
      return ev;
    }
    bool sLe(const ExprValue& Rhs) const {
      return static_cast<int32_t>(intVal.uintValue) <= static_cast<int32_t>(Rhs.intVal.uintValue);
    }
    bool uLe(const ExprValue& Rhs) const {
      return intVal.uintValue <= Rhs.intVal.uintValue;
    }

    ExprValue neq(const ExprValue& Rhs) const{
      assert((valueType != kFloatTy) && (Rhs.valueType != kFloatTy) && "not equal to operator is not suitable for FLOAT");
      assert((valueType == Rhs.valueType) && "unequal operand type must be same");
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kIntTy){
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.uintValue != Rhs.intVal.uintValue;
      } else {
        ev.boolVal = boolVal != Rhs.boolVal;
      }
      return ev;
    }

    ExprValue equal(const ExprValue& Rhs) const{
      assert((valueType != kFloatTy) && (Rhs.valueType != kFloatTy) && "equal to operator is not suitable for FLOAT" );
      assert((valueType == Rhs.valueType) && "equal operand type must be same");
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kIntTy){
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = intVal.uintValue == Rhs.intVal.uintValue;
      } else {
        ev.boolVal = boolVal == Rhs.boolVal;
      }
      return ev;
    }

    ExprValue bitAnd(const ExprValue& Rhs) const{ //todo not overloading operator this prevent forward declaration use a function
      assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise AND operator is only suitable for INT" );
      assert((!isNegative() && !Rhs.isNegative() ) && "bitwise AND operand must be greater than or equal to 0");
      ExprValue ev;
      ev.valueType = kIntTy;
      assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
      ev.intVal.uintValue = intVal.uintValue & Rhs.intVal.uintValue;
      return ev;
    }

    ExprValue bitXor(const ExprValue& Rhs) const{
      assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise XOR operator is only suitable for INT" );
      //assert((Rhs.intVal.uintValue >= 0) && (intVal.uintValue >= 0) && "bitwise XOR operands must be greater than or equal to 0");
      ExprValue ev;
      ev.valueType = kIntTy;
      assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
      ev.intVal.uintValue = intVal.uintValue ^ Rhs.intVal.uintValue;
      return ev;
    }

    ExprValue bitOr(const ExprValue& Rhs) const{
      assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise OR operator is only suitable for INT" );
      //assert((Rhs.intVal.uintValue >= 0) && (intVal.uintValue >= 0) && "bitwise OR operand must be greater than or equal to 0");
      ExprValue ev;
      ev.valueType = kIntTy;
      assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
      ev.intVal.uintValue = intVal.uintValue | Rhs.intVal.uintValue;
      return ev;
    }

    ExprValue logicalAnd(const ExprValue& Rhs) const{
      assert((valueType != kFloatTy) && (Rhs.valueType != kFloatTy) && "logical AND operator is not suitable for FLOAT" );
      assert((valueType == Rhs.valueType) && "logical AND operands type must be same");
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kIntTy){
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = (intVal.uintValue && Rhs.intVal.uintValue);
      } else {
        ev.boolVal = (boolVal && Rhs.boolVal);
      }
      return ev;
    }

    ExprValue logicalOr(const ExprValue& Rhs) const{
      assert((valueType != kFloatTy) && (Rhs.valueType != kFloatTy) && "logical OR operator is not suitable for FLOAT" );
      assert((valueType == Rhs.valueType) && "logical OR operands type must be same");
      ExprValue ev;
      ev.valueType = kBoolTy;
      if (valueType == kIntTy){
        assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
        ev.boolVal = (intVal.uintValue || Rhs.intVal.uintValue);
      } else {
        ev.boolVal = (boolVal || Rhs.boolVal);
      }
      return ev;
    }

  private:
    struct IntTy{
      bool isSigned;
      uint32_t uintValue;
    };
    bool evaluatable;
    TyKinds valueType;
    union{
      bool boolVal;
      IntTy intVal;
      float floatVal;
    };
    friend class FDSin;
    friend class FDCos;
    friend class FDTg;
    friend class FDCtg;
    friend class FDLog;
    friend class FDPow;
    friend class FDMax;
    friend class FDMin;
    friend class FDAbs;
    friend class FDSqrt;
    friend class ExprNode;
    friend class Constant;
    friend class CastExpr;
    friend class ArraySubscriptExpr;
    friend class Sematic;
};

struct Type{
  virtual MString getTypeLiteral() const = 0;
  virtual TyKinds getKind() const       = 0;
  virtual uint32_t getTypeWidth() const = 0;
  virtual bool isIntegerType() const    = 0;
  virtual bool isFloatingType() const   = 0;
  virtual bool isBooleanType() const    = 0;
  virtual bool isArrayElement() const   = 0;
  virtual bool isSigned() const = 0;
  virtual ~Type() {}
};

class DeclType : public Type {
  public:
    explicit DeclType(bool signd, uint32_t width,
            TyKinds ty, const MString& literal)
      :signedness_(signd)
      ,typeWidth_(width)
      ,typeKind_(ty)
      ,typeLiteral_(literal){}
    virtual ~DeclType(){}
    virtual MString getTypeLiteral() const { return typeLiteral_; }
    virtual TyKinds getKind() const       { return typeKind_; }
    virtual uint32_t getTypeWidth() const { return typeWidth_*sizeof(char)*8U; } //FIXME : rename to mean bits
    virtual bool isIntegerType() const    { return typeKind_ == kIntTy;   }
    virtual bool isBooleanType() const    { return typeKind_ == kBoolTy;  }
    virtual bool isFloatingType() const   { return typeKind_ == kFloatTy; }
    virtual bool isArrayElement() const   { return false; }
    virtual bool isSigned() const         { return signedness_; }
    void SetSignedness(const bool signedness) { signedness_ = signedness; }
  private:
//    false 0 0.0
    bool signedness_;
    uint32_t typeWidth_;
    TyKinds typeKind_;
    MString typeLiteral_;
};

class ExprResult;
class QualType{
  public:
    QualType()
      :pType_(NULL) {}
    explicit QualType(Type* type)
      :pType_(type) {}
    ~QualType()     {}
    Type* get() const { return pType_; }
    void release() { SAFE_DELETE(pType_); }
    bool isIntegralPromotion(const QualType& toType);

    bool isIntegerType() const { return pType_->isIntegerType(); }
    bool isSignedInteger() const  { return pType_->isSigned();  }
    uint32_t getTypeWidth() const { return pType_->getTypeWidth(); }

    bool isArithmeticType(){
      return pType_->isIntegerType() || pType_->isFloatingType();
    }
    bool isArrayElement() const   { return pType_->isArrayElement();}
    bool hasIntegerRepresentation(){
      return pType_->isIntegerType() || pType_->isBooleanType();
    }
    bool isNull() const     { return NULL == pType_; }
    bool isEqual(const QualType& rhs) const   { return pType_ == rhs.pType_;  } //todo compare type content rethink
    void setArraySize(const int32_t newSize); 
    ExprResult createDefaultInit(IdentifierTable& table);//todo
  private:
    Type *pType_;
};

class ConstArrayType : public Type{
  public:
    explicit ConstArrayType(QualType& element, int32_t size)
      : elemBase_(element), arraySize_(size) {}
    virtual ~ConstArrayType() {}
    virtual MString getTypeLiteral() const;
    virtual TyKinds getKind() const { return elemBase_.get()->getKind(); }
    virtual uint32_t getTypeWidth() const { return elemBase_.get()->getTypeWidth();}
    virtual bool isIntegerType() const    { return elemBase_.get()->isIntegerType();}
    virtual bool isFloatingType() const   { return elemBase_.get()->isFloatingType();}
    virtual bool isBooleanType() const    { return elemBase_.get()->isBooleanType();}
    virtual bool isArrayElement() const   { return true;}
    virtual bool isSigned() const         { return false; }//todo

    int32_t getArraySize() const { return arraySize_; }
    void setArraySize(const int32_t newSize)  { arraySize_ = newSize; }
  private:
    QualType elemBase_;
    int32_t arraySize_;
};

enum CastKind { //todo
  kCastNoOp,
  kCastIntegral,
  kCastFloatingToIntegral,
  kCastFloatingToBoolean,
  kCastIntegralToFloating,
  kCastIntegralToBoolean,
  kCastIncompatible
};

class ExprNode{
  public:

    enum StmtClass{
      kBinaryOperaotrExprClass,
      kCastExprClass,
      kUnaryOperatorExprClass,
      kConstantClass,
      kArraySubscriptExprClasss,
      kBuiltinCallExprClass,
      kVarDeclClass,
      kInitExprsClass,
      kParenExpr
    };

    explicit ExprNode(QualType& T)
       :resultTy_(T){}
    explicit ExprNode(StmtClass sc)
      :resultTy_(NULL), exprClass_(sc) {}
    explicit ExprNode(QualType T, StmtClass sc)
      :resultTy_(T), exprClass_(sc) {}
    ~ExprNode() { resultTy_.release(); }

    virtual ExprValue evaluate()        { return ExprValue();}
    virtual ExprNode* getBase() const   { return NULL;    }
    virtual ExprNode* getIdx() const    { return NULL;    }
    virtual CastKind getCastKind() const { return kCastNoOp; }
    QualType getQualType()const { return resultTy_;}
    QualType getQualType() { return resultTy_;}
    void setResultType(const QualType& resultType){ resultTy_ = resultType; }
    bool isArrayVar() const     { return false;}
    bool isConstantZero() const { return false; }
    StmtClass getExprClass() const { return static_cast<StmtClass>(exprClass_); }
    void success(ExprValue& resultVal);
    virtual std::vector<ExprValue> getInitValueList() { //todo
      std::vector<ExprValue> m;
      m.clear();
      return m;
    }
  private:
    QualType resultTy_;
    uint16_t exprClass_;
    DISALLOW_COPY_AND_ASSIGN(ExprNode);
};

class ExprResult{
  public:
    explicit ExprResult(ExprNode* ptr)
      :ptrTy_(ptr)
      ,isInvalid_(false){}
    explicit ExprResult(const bool invalid)
      :ptrTy_(NULL)
      ,isInvalid_(invalid){}
    //copy ctor
    ExprResult(const ExprResult& rhs)
      :ptrTy_(rhs.ptrTy_)
      ,isInvalid_(rhs.isInvalid_){}
    ~ExprResult() { ptrTy_ = NULL; isInvalid_ = true; }

    ExprNode* get() const { return ptrTy_; }
    ExprNode* move() const { return get(); }
    bool isInvalid() const { return isInvalid_; }
    ExprValue evaluate() { assert(ptrTy_); return ptrTy_->evaluate(); }

  private:
    ExprNode* ptrTy_;
    bool isInvalid_;
};

inline ExprResult ExprError(){ return ExprResult(true); }

enum OperatorKind {
  kBOMul,   kBODiv,     kBORem,     kBOAdd,
  kBOSub,   kBOShl,     kBOShr,     kBOLE,
  kBOLT,    kBOGE,      kBOGT,      kBONE,
  kBOEQ,    kBOBitAnd,  kBOBitXor,  kBOBitOr, 
  kBOLAnd,  kBOLOr,     kBOAssign,  kBOComma,
  kUOPlus,  kUOMinus,   kUONot,     kUOLNot
};

class BinaryOperatorExpr : public ExprNode{
  public:
    explicit BinaryOperatorExpr(ExprResult& lhs, ExprResult& rhs, OperatorKind opk, QualType& resultTy)
      :ExprNode(resultTy, kBinaryOperaotrExprClass)
      ,lhs_(lhs.get())
      ,rhs_(rhs.get())
      ,opk_(opk){};
    virtual ExprValue evaluate();
    typedef ExprValue (BinaryOperatorExpr::*visitBinOp)();
  private:
    static std::map<OperatorKind, visitBinOp> binOpFunctors_;
    static std::map<OperatorKind, visitBinOp> initializeBinOpMap();
    ExprValue visitMulOpExpr();
    ExprValue visitAddOpExpr();
    ExprValue visitSubOpExpr();
    ExprValue visitDivOpExpr();
    ExprValue visitRemOpExpr();
    ExprValue visitShlOpExpr();
    ExprValue visitShrOpExpr();
    ExprValue visitAssignOpExpr();
    ExprValue visitLEOpExpr();
    ExprValue visitLTOpExpr();
    ExprValue visitGEOpExpr();
    ExprValue visitGTOpExpr();
    ExprValue visitEQOpExpr();
    ExprValue visitNEOpExpr();
    ExprValue visitBitwiseAndOpExpr();
    ExprValue visitBitwiseXorOpExpr();
    ExprValue visitBitwiseOrOpExpr();
    ExprValue visitLogicalAndOpExpr();
    ExprValue visitLogicalOrOpExpr();
    ExprResult lhs_;
    ExprResult rhs_;
    OperatorKind opk_;
};

class Constant : public ExprNode{
  public:
    explicit Constant(uint32_t val, QualType T)
      :ExprNode(T, kConstantClass) {
      constValue_.valueType = kIntTy;
      constValue_.intVal.isSigned = T.get()->isSigned();
      constValue_.intVal.uintValue = val;
    }
    explicit Constant(float val, QualType T)
      :ExprNode(T, kConstantClass) {
      constValue_.valueType = kFloatTy;
      constValue_.floatVal = val;
    }
    explicit Constant(bool val, QualType T)
      :ExprNode(T, kConstantClass) {
      constValue_.valueType = kBoolTy;
      constValue_.boolVal = val;
    }
    bool isConstantZero() const {
      return constValue_.isZero();
    }
    virtual ExprValue evaluate(){
      return constValue_;
    }
  private:
    ExprValue constValue_;
};

class UnaryOperatorExpr : public ExprNode{
  public:
    explicit UnaryOperatorExpr(ExprNode* Input, OperatorKind Kind, QualType& resultType)
      :ExprNode(resultType, kUnaryOperatorExprClass)
      ,Input_(Input)
      ,Opc_(Kind) {}

    virtual ExprValue evaluate();
  private:
    ExprNode* Input_;
    OperatorKind Opc_;
};


class CastExpr : public ExprNode{
  public:
    explicit CastExpr(QualType& Ty, CastKind Kind, ExprResult SubExpr)
      :ExprNode(Ty, kCastExprClass)
      ,CKind_(Kind)
      ,OpExpr_(SubExpr) {}

    virtual CastKind getCastKind() const { return CKind_; }
    virtual ExprValue evaluate();
  private:

    ExprValue handleIntToIntCast(const ExprValue& subValue);
    ExprValue handleConversionToBool(const ExprValue& subValue);
    ExprValue handleFloatToIntCast(const ExprValue& subValue);
    ExprValue handleIntToFloatCast(const ExprValue& subValue);
    CastKind CKind_;
    ExprResult OpExpr_;
};

class FunctionDecl;
typedef std::vector<ExprResult> ExprVector;
class BuiltinCallExpr : public ExprNode{
  public:
    explicit BuiltinCallExpr(FunctionDecl* pFD)
      :ExprNode(kBuiltinCallExprClass) // QualType set in analyzeBuiltinCallExpr
      ,pFnDecl_(pFD) {
      arguments_.clear();
    }
    void setArg(const int idx, ExprResult& arguE) {
      //arguments_[idx] = arguE;
      arguments_.push_back(arguE);
    }
    QualType getArgType(const int idx) {
      return arguments_[idx].get()->getQualType();
    }
    FunctionDecl* getFuncDecl() {
      return pFnDecl_;
    }
    virtual ExprValue evaluate();

  private:
    //ExprNode* lhsFn_;
    ExprVector arguments_;
    //QualType resultTy_;
    FunctionDecl* pFnDecl_;
};

class ArraySubscriptExpr : public ExprNode{
  public:
    ArraySubscriptExpr(ExprNode* baseExpr, ExprNode* idxExpr, QualType& resultTy)
      :ExprNode(resultTy, kArraySubscriptExprClasss)
       ,baseExpr_(baseExpr)
       ,indexExpr_(idxExpr){};
    virtual ExprNode* getBase() const   { return baseExpr_;    }
    virtual ExprNode* getIdx() const    { return indexExpr_;    }

    virtual ExprValue evaluate();
  private:
    ExprNode* baseExpr_;
    ExprNode* indexExpr_;
};

class InitExprs: public ExprNode {
  public:
    explicit InitExprs(std::vector<ExprNode*>& initExprNodes)
      :ExprNode(kInitExprsClass), initNodes_(initExprNodes) {}
    virtual std::vector<ExprValue> getInitValueList() {
      std::vector<ExprValue> exprValVec;
      std::vector<ExprNode*>::iterator it = initNodes_.begin();
      for(; it != initNodes_.end(); ++it)
        exprValVec.push_back((*it)->evaluate() );
      return exprValVec;
    }
    const std::vector<ExprNode*>& getInitNodeList() const { return initNodes_; }

  private:
    std::vector<ExprNode*> initNodes_;
};

class VarDecl : public ExprNode {
  public:
    explicit VarDecl(QualType& T)
      :ExprNode(T, kVarDeclClass), pVarValue_(NULL) {}

    void setInitialier(const ExprResult& initialier) {
      if (isArrayVar()) {
        InitExprs *pInitializer = static_cast<InitExprs*>(initialier.get());
        const std::vector<ExprNode*>& exprList = pInitializer->getInitNodeList();
        getQualType().setArraySize(exprList.size());
      }
      pVarValue_ = initialier.move();//todo checkAssignmentOperands Constant with Type??
    }
    bool isArrayVar() const {
      return getQualType().isArrayElement();
    }
    virtual std::vector<ExprValue> getInitValueList() {
      assert(isArrayVar() );
      return pVarValue_->getInitValueList();
    }
    virtual ExprValue evaluate();
  private:
    ExprNode* pVarValue_;
};

class ParenExpr : public ExprNode {
  public:
    ParenExpr(ExprResult& exp)
      :ExprNode(exp.get()->getQualType(), kParenExpr)
      ,exp_(exp) {}

    ExprValue evaluate() { return exp_.get()->evaluate(); }

  private:
    ExprResult exp_;
};

}
}

#endif /* AST_H_199850DF_581D_4723_BCBF_33E06EE91387 */
