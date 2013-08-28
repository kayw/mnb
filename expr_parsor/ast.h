namespace mnb{
namespace expr{
#define MAX_BITS 32
enum TyKinds{
  kVoidTy,  kBoolTy,
  kIntTy,   kFloatTy,
};
struct ExprValue{
  ExprValue()
    :evaluatable(false)
    ,valueType(kVoidTy){}

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
  }
  bool isNegative() const {
    if (valueType == kIntTy) {
      return intVal.isSigned && (static_cast<int32_t>intVal.uintValue < 0);
    //} else if (valueType == kFloatTy) {
    //  return floatVal < 0;
    } else{
      assert(false && "Value type must be INT");
    }
  }
  int32_t getValidBits() const{//todo
    if(valueType != kIntTy)
      return 0;
    uint32_t tmp = 0
    for (int i = MAX_BITS; i >= 0 ; --i) {
      tmp = intVal.uintValue&(~uint32_t(0UL) >> (MAX_BITS - width));
      if(tmp == 1)
        return i;
    }
  }
    
  bool isTrue() const {
  }
  void truncToWidth(const int width) {
    if (valueType != kIntTy)
      return;
    if (width > MAX_BITS)
      width = MAX_BITS;
    intVal.uintValue &= ~uint32_t(0UL) >> (MAX_BITS - width);
  }
  //UnaryOperator
  ExprValue operator-() const{
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
  ExprValue operator!() const{
    assert((valueType == kBoolTy) && "logical Not operator is only for BOOL" );
    ExprValue ret;
    ret.valueType = kBoolTy;
    ret.boolVal = !boolVal;
    return ret;
  }
  ExprValue operator*(const ExprValue& Rhs) const { //unsigned cast signed
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
      ev.intVal.isSigned == intVal.isSigned;
      ev.intVal.uintValue = intVal.uintValue * Rhs.intVal.uintValue;
    }
    return ev;
  }

  ExprValue operator+(const ExprValue& Rhs) const {
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
      ev.intVal.isSigned == intVal.isSigned;
      ev.intVal.uintValue = intVal.uintValue + Rhs.intVal.uintValue;
    }
    return ev;
  }

  ExprValue operator-(const ExprValue& Rhs) const {
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
      ev.intVal.isSigned == intVal.isSigned;
      ev.intVal.uintValue = intVal.uintValue - Rhs.intVal.uintValue;
    }
    return ev;
  }
  ExprValue operator/(const ExprValue& Rhs) const {
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
      ev.intVal.isSigned == intVal.isSigned;
      ev.intVal.uintValue = intVal.isSigned ? sDiv(Rhs) : uDiv(Rhs);intVal / Rhs.intVal;
    }
    return ev;
  }
  uint32_t sDiv(const ExprValue& Rhs) const {
    assert((Rhs.intVal.uintValue == 0) && "divide by zero?");
    return uint32_t(static_cast<int32_t>(intVal.uintValue) / static_cast<int32_t>(Rhs.intVal.uintValue) );
  }
  uint32_t uDiv(const ExprValue& Rhs) const {
    assert((Rhs.intVal.uintValue == 0) && "divide by zero?");
    return intVal.uintValue / Rhs.intVal.uintValue;
  }
  uint32_t sRem(const ExprValue& Rhs) const {
    assert((Rhs.intVal.uintValue == 0) && "remainder by zero?");
    return uint32_t(static_cast<int32_t>(intVal.uintValue)/static_cast<int32_t>(Rhs.intVal.uintValue) );
  }
  uint32_t uRem(const ExprValue& Rhs) const {
    assert((Rhs.intVal.uintValue == 0) && "remainder by zero?");
    return intVal.uintValue % Rhs.intVal.uintValue;
  }
  ExprValue operator%(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType != kIntTy) && "modula operator is only suitable for INT" );
    ExprValue ev;
    ev.valueType = kIntTy;
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    ev.intVal.isSigned == intVal.isSigned;
    ev.intVal.uintValue = intVal.isSigned ? sRem(Rhs) : uRem(Rhs);intVal % Rhs.intVal;
    return ev;
  }
  ExprValue operator<<(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType != kIntTy) && "right shift operator is only suitable for INT" );
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    assert((!isNegative() && !Rhs.isNegative() ) && "right shift operands must be greater than or equal to 0")
    ExprValue ev;
    ev.valueType = kIntTy;
    uint32_t shiftBits = Rhs.intVal;
    if (shiftBits >= MAX_BITS){
      ev.intVal = intVal;
    } else {
      ev.intVal.isSigned = intVal.isSigned;
      ev.intVal.uintValue = intVal.uintValue << Rhs.intVal.uintValue;
    }
    return ev;
  }
  ExprValue operator>>(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType != kIntTy) && "left shift operator is only suitable for INT" );
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    assert((!isNegative() && !Rhs.isNegative() ) && "left shift operands must be greater than or equal to 0");
    ExprValue ev;
    ev.valueType = kIntTy;
    ev.intVal.isSigned = intVal.isSigned;
    ev.intVal.uintValue = intVal.isSigned ? sShr(Rhs.intVal.uintValue) : uShr(Rhs.intVal.uintValue);//intVal >> Rhs.intVal;
    return ev;
  }
  ExprValue operator>(const ExprValue& Rhs) const{
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
  bool sGe(const ExprValue& Rhs) const {
    return static_cast<int32_t>(intVal.uintValue) >= static_cast<int32_t>(Rhs.intVal.uintValue);
  }
  bool uGe(const ExprValue& Rhs) const {
    return intVal.uintValue >= Rhs.intVal.uintValue;
  }
  bool sLt(const ExprValue& Rhs) const {
    return static_cast<int32_t>(intVal.uintValue) < static_cast<int32_t>(Rhs.intVal.uintValue);
  }
  bool uLt(const ExprValue& Rhs) const {
    return intVal.uintValue < Rhs.intVal.uintValue;
  }
  bool sLe(const ExprValue& Rhs) const {
    return static_cast<int32_t>(intVal.uintValue) <= static_cast<int32_t>(Rhs.intVal.uintValue);
  }
  bool uLe(const ExprValue& Rhs) const {
    return intVal.uintValue <= Rhs.intVal.uintValue;
  }
  ExprValue operator>=(const ExprValue& Rhs) const{
    assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "greater than or equal to operator is not suitable for BOOL" );
    ExprValue ev;
    ev.valueType = kBoolTy;
    if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
      ev.boolVal = (floatVal > Rhs.floatVal) || fabs(floatVal - Rhs.floatVal) < std::numeric_limits::epsilon;
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
  ExprValue operator<(const ExprValue& Rhs) const{
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
  ExprValue operator<=(const ExprValue& Rhs) const{
    assert((valueType != kBoolTy) && (Rhs.valueType != kBoolTy) && "less than or equal to operator is not suitable for BOOL" );
    ExprValue ev;
    ev.valueType = kBoolTy;
    if (valueType == kFloatTy && Rhs.valueType == kFloatTy ){
      ev.boolVal = floatVal < Rhs.floatVal || fabs(floatVal - Rhs.floatVal) < std::numeric_limits::epsilon;
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
  ExprValue operator!=(const ExprValue& Rhs) const{
    assert((valueType != kFloatTy) && (Rhs.valueType != kFloatTy) && "not equal to operator is not suitable for FLOAT" );
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
  ExprValue operator==(const ExprValue& Rhs) const{
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
  ExprValue operator&(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise AND operator is only suitable for INT" );
    assert((!isNegative() && !Rhs.isNegative() ) && "bitwise AND operand must be greater than or equal to 0")
    ExprValue ev;
    ev.valueType = kIntTy;
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    ev.intVal.uintValue = intVal.uintValue & Rhs.intVal.uintValue;
    return ev;
  }
  ExprValue operator^(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise XOR operator is only suitable for INT" );
    assert((Rhs.intVal >= 0) && (intVal >= 0) "bitwise XOR operands must be greater than or equal to 0")
    ExprValue ev;
    ev.valueType = kIntTy;
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    ev.intVal.uintValue = intVal.uintValue ^ Rhs.intVal.uintValue;
    return ev;
  }
  ExprValue operator|(const ExprValue& Rhs) const{
    assert((valueType == kIntTy) && (Rhs.valueType == kIntTy) && "bitwise OR operator is only suitable for INT" );
    assert((Rhs.intVal >= 0) && (intVal >= 0) "bitwise OR operand must be greater than or equal to 0")
    ExprValue ev;
    ev.valueType = kIntTy;
    assert((intVal.isSigned == Rhs.intVal.isSigned) && "signedness mismatch");
    ev.intVal = intVal.uintValue | Rhs.intVal.uintValue;
    return ev;
  }

  ExprValue operator&&(const ExprValue& Rhs) const{
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
  ExprValue operator||(const ExprValue& Rhs) const{
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
};
class Node{
};
struct Type{
  TyKinds getKind() const       = 0;
  uint32_t getTypeWidth() const = 0;
  bool isIntegerType() const    = 0;
  bool isFloatingType() const   = 0;
  bool isBooleanType() const    = 0;
  bool isArrayElement() const   = 0;
};

class DeclType : public Type{
  public:
    DeclType(bool signd,uint32_t width,
            TyKinds ty, const string& literal)
      :signedness_(signd)
      ,typeWidth_(width)
      ,typeKind_(ty)
      ,typeLiteral_(literal){}
    TyKinds getKind() const       { return typeKind_; }
    uint32_t getTypeWidth() const { return typeWidth_*sizeof(char); }
    bool isIntegerType() const    { return typeKind_ == kIntTy; }
    bool isBooleanType() const    { return typeKind_ == kBoolTy; }
    bool isFloatingType() const   { return typeKind_ == kFloatTy; }
    bool isArrayElement() const   { return false;}
  private:
//    false 0 0.0
    bool signedness_;
    uint32_t typeWidth_;
    TyKinds typeKind_;
    string typeLiteral_;
};

class QualType{
  public:
    QualType(Type* type)
      :pType_(type){}
    Type* get() { return pType_; }
    void release() { SAFE_DELETE(pType_); }
    bool isIntegralPromotion(const QualType& toType);

    bool isIntegerType() const;
    bool isSignedInteger() const;

    bool isArithmeticType(){
      return pType_->isIntegerType() || pType_->isFloatingType();
    }
    bool hasIntegerRepresentation(){
      return pType_->isIntegerType() || pType_->isBooleanType();
    }
    ExprResult createDefaultInit();
  private:
    Type *pType_;
};

class ConstArrayType : public Type{
  public:
    ConstArrayType(QualType element, int32_t size);
    ~ConstArrayType();
    TyKinds getKind() const { return elemBase_.get()->getKind(); }
    uint32_t getTypeWidth() const { return elemBase_.get()->getTypeWidth();}
    bool isIntegerType() const    { return elemBase_.get()->isIntegerType();}
    bool isFloatingType() const   { return elemBase_.get()->isFloatingType();}
    bool isBooleanType() const    { return elemBase_.get()->isBooleanType();}
    bool isArrayElement() const   { return true;}

    int32_t getArraySize() const { return arraySize_; }
  private:
    QualType elemBase_;
    int32_t arraySize_;
};

class ExprResult{
  public:
    ExprResult(ExprResult rhs)
      :ptrTy_(rhs.ptrTy_)
      ,isInvalid_(rhs.isInvalid_){}

    ExprResult(ExprNode* ptr)
      :ptrTy_(ptr)
       ,isInvalid_(false){}
    ExprResult(bool invalid)
      :isInvalid_(invalid)
       ,ptrTy_(NULL){}
    ExprNode* get() const { return ptrTy_; }
    ExprNode* move() const { return get(); }
    bool isInvalid_() const { return isInvalid_; }

  private:
    ExprNode* ptrTy_;
    bool isInvalid_;
};

class ExprNode{
  public:
    ExprNode(QualType T)
       :resultTy_(T){}
    ExprValue evaluate(){return ExprValue();}
    QualType getQualType(){ return resultTy_;}
    bool isArrayVar() const {return false;}
    std::vector<ExprValue> getInitValueList() {
      std::vector<ExprValue> m;
      m.clear();
      return m;
    }
    enum StmtClass{
      kBinaryOperaotrExprClass,
      kCastExprClass,
      kUnaryOperatorExprClass,
      kConstantClass,
      kArraySubscriptExprClasss,
      kBuiltinCallExprClass,
      kVarDeclClass,
      kInitExprsClass
    };
  private:
    QualType resultTy_;
    uint16_t exprClass_;
};

class BinaryOperatorExpr : public ExprNode{
  public:
    BinaryOperatorExpr(const ExprResult& lhs, const ExprResult& rhs, OperatorKind opk, QualType resultTy)
      :ExprNode(resultTy)
      ,exprClass_(kBinaryOperaotrExprClass)
      ,Lhs_(lhs)
      ,Rhs(rhs)
      ,Opk_(opk){};
    typedef ExprValue (*visitBinOp)(ExprResult lhs, ExprResult rhs);
  private:
    static const std::map<OperatorKind, visitBinOp> BinOpFunctors_;
    static std::map<OperatorKind, visitBinOp> initializeBinOpMap();
    const ExprResult& Lhs_;
    const ExprResult& Rhs_;
    OperatorKind Opk_;
};

class Constant : public ExprNode{
  public:
    explicit Constant(uint32_t val){
      exprClass_ = kConstantClass;
      constValue_.valueType = kIntTy;
      constValue_.intVal.isSigned = false;
      constValue_.intVal.uintValue = val;
    }
    explicit Constant(float val){
      exprClass_ = kConstantClass;
      constValue_.valueType = kFloatTy;
      constValue_.floatVal = val;
    }
    explicit Constant(bool val){
      exprClass_ = kConstantClass;
      constValue_.valueType = kBoolTy;
      constValue_.boolVal = val;
    }
    ExprValue evaluate(){
      return constValue_;
    }
  private:
    ExprValue constValue_;
};

class UnaryOperatorExpr : public ExprNode{
  public:
    UnaryOperatorExpr(const ExprNode* Input, UnaryOperatorKind Kind, QualType resultType)
      :ExprNode(resultType)
      ,Input_(Input)
      ,Opc_(Kind)
      ,exprClass_(kUnaryOperatorExprClass){}

  private:
    ExprNode* Input_;
    UnaryOperatorKind Opc_;
};

class CastExpr : public ExprNode{
  public:
    CastExpr(const QualType Ty, CastKind Kind, ExprResult SubExpr)
      :ExprNode(Ty)
      ,CKind_(Kind)
      ,OpExpr_(SubExpr)
      ,exprClass_(kCastExprClass){}

  private:
    CastKind CKind_;
    ExprResult OpExpr_;
}

class BuiltinCallExpr : public ExprNode{
  public:
    BuiltinCallExpr(FunctionDecl* pFD)
    : fnDecl_(pFD)
    , exprClass_(kBuiltinCallExprClass){
      arguments_.clear();
    }
    void setResultType(const QualType& resultType){
      resultTy_ = resultType;
    }
    void setArg(const int idx, ExprResult& arguE){
      arguments_[idx] = arguE;
    }

  private:
    //ExprNode* lhsFn_;
    ExprVector arguments_;
    QualType resultTy_;
    FunctionDecl* fnDecl_;
};

class ArraySubscriptExpr : public ExprNode{
  public:
    ArraySubscriptExpr(ExprNode* baseExpr, ExprNode* idxExpr, QualType& resultTy)
      :ExprNode(resultTy)
       ,baseExpr_(baseExpr)
       ,indexExpr_(idxExpr){};

  private:
    ExprNode* baseExpr_;
    ExprNode* indexExpr_;
};

class VarDecl : public ExprNode{
  public:
    VarDecl( QualType T)
      : ExprNode(T)
      ,VarValue_(NULL){}
    void setInitialier(const ExprResult& initialier) {
      VarValue_ = initialier.release();
    }
    bool isArrayVar() const {
      return resultTy_->isArrayElement();
    }
    std::vector<ExprValue> getInitValueList() {
      assert(isArrayVar() );
      return VarValue_.getInitValueList();
    }
  private:
    ExprResult VarValue_;
};

class InitExprs: public ExprNode {
  public:
    InitExprs(std::vector<ExprNode*>& initExprNodes)
      :initNodes_(initExprNodes)
      ,exprClass_(kInitExprsClass) {}
    std::vector<ExprValue> getInitValueList() {
      std::vector<ExprValue> exprValVec;
      std::vector<ExprNode*>::const_iterator cit = initNodes_.begin();
      for(; cit != initNodes_.end(); ++cit)
        exprValVec.push_back(cit->evaluate() );
    }

  private:
    std::vector<ExprNode*>& initNodes_;
}

class ParenExpr : public ExprNode {
  public:
    ParenExpr(ExprResult& exp)
      :exp_(exp){}

    ExprValue evaluate() { return exp_.get()->evaluate(); }

  private:
    ExprResult exp_;
};

class Logical : public ExprNode{
  public:
    Logical(Token* pTok, ExprNode x1, ExprNode x2)
      : ExprNode(pTok, null_type){
        left_expr_ = x1;
        right_expr_ = x2;
        type_ = checkType(x1.type_, x2.type_);
      }
    Type checkType(const Type& t1, const Type& t2){
      if (t1 == bool && t2 == bool) {
        return bool;
      }
      else{
        return null_type;
      }
    }
  private:
    ExprNode left_expr_;
    ExprNode right_expr_;
};
class Or : public Logical{
  public:
    Or(Token* pTok, ExprNode x1, ExprNode x2)
      : Logical(pTok, x1, x2){}
};
class And : public Logical{
  public:
    And(Token* pTok, ExprNode x1, ExprNode x2)
      : Logical(pTok, x1, x2){}
};
class Not : public Logical{
  public:
    Not(Token* pTok, ExprNode x1, ExprNode x2)
      : Logical(pTok, x1, x2){}
};
class Relative : public Logical{
  public:
    Relative(Token* pTok, ExprNode x1, ExprNode x2)
      : Logical(pTok, x1, x2){}
    Type checkType(const Type& t1, const Type& t2){
      if (t1 == Array || t2 == Array) {
        return null_type;
      }
      else if (t1 == t2) {
        return bool;
      }
      else{
        return null_type;
      }
    }
};
class Array1D : public ExprNode{
  public:
    Array1D(ID id, ExprNode x, Type p)
      : ExprNode(new Word("[]", Token::), p){
        array_ = id;
        index_ = x;
      }
  private:
    ID array_;
    ExprNode index_;
};
}
}
