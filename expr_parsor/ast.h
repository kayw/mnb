#ifndef AST_H_199850DF_581D_4723_BCBF_33E06EE91387
#define AST_H_199850DF_581D_4723_BCBF_33E06EE91387

#include <map>
#include <vector>
#include <limits.h>
#include "mmacros.h"
#include "mstring.h"
#include "expr_value.h"

namespace mnb {
namespace expr {

class IdentifierTable;

struct Type {
  virtual MString getTypeLiteral() const = 0;
  virtual TyKinds getKind() const       = 0;
  virtual uint32_t getTypeBits() const = 0;
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
    virtual uint32_t getTypeBits() const { return typeWidth_*sizeof(char)*CHAR_BIT; }
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
class ExprNode;
class QualType {
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
    uint32_t getTypeBits() const { return pType_->getTypeBits(); }

    bool isArithmeticType(){
      return pType_->isIntegerType() || pType_->isFloatingType();
    }
    bool isArrayElement() const   { return pType_->isArrayElement();}
    bool hasIntegerRepresentation(){
      return pType_->isIntegerType() || pType_->isBooleanType();
    }
    bool isNull() const     { return NULL == pType_; }
    bool isSameQualType(const QualType& rhs) const;
    void setArraySize(const std::vector<ExprNode*>* pInitList); 
    ExprResult createDefaultInit(IdentifierTable& table);
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
    virtual uint32_t getTypeBits() const { return elemBase_.get()->getTypeBits();}
    virtual bool isIntegerType() const    { return elemBase_.get()->isIntegerType();}
    virtual bool isFloatingType() const   { return elemBase_.get()->isFloatingType();}
    virtual bool isBooleanType() const    { return elemBase_.get()->isBooleanType();}
    virtual bool isArrayElement() const   { return true;}
    virtual bool isSigned() const         { return elemBase_.get()->isSigned(); }

    int32_t getArraySize() const { return arraySize_; }
    void setArraySize(const int32_t newSize, const ExprNode* pSubInit); 
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
    QualType getQualType()const { return resultTy_;} //todo: return underlying type.i.e array type is element base's QualType
    QualType getQualType() { return resultTy_;}
    void setResultType(const QualType& resultType){ resultTy_ = resultType; }
    bool isArrayVar() const     { return false;}
    bool isConstantZero() const { return false; }
    StmtClass getExprClass() const { return static_cast<StmtClass>(exprClass_); }
    void success(ExprValue& resultVal);
  private:
    QualType resultTy_;
    uint16_t exprClass_;
    DISALLOW_COPY_AND_ASSIGN(ExprNode);
};

class ExprResult{ //todo remove this unneed wrapper
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
    void setArg(const int /*idx*/, ExprResult& arguE) {
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

class InitExprs;
class ArraySubscriptExpr : public ExprNode{
  public:
    ArraySubscriptExpr(ExprNode* baseExpr, ExprNode* idxExpr, QualType& resultTy)
      :ExprNode(resultTy, kArraySubscriptExprClasss)
       ,baseExpr_(baseExpr)
       ,indexExpr_(idxExpr){};
    virtual ExprNode* getBase() const   { return baseExpr_;    }
    virtual ExprNode* getIdx() const    { return indexExpr_;    }

    virtual ExprValue evaluate();
    InitExprs* getLowRankInitial();
  private:
    ExprNode* baseExpr_;
    ExprNode* indexExpr_;
};

class InitExprs: public ExprNode {
  public:
    explicit InitExprs(QualType T, std::vector<ExprNode*>& initExprNodes)
      :ExprNode(T, kInitExprsClass), initNodes_(initExprNodes) {}
    const std::vector<ExprNode*>* getInitExprVec() const { return &initNodes_; }

  private:
    std::vector<ExprNode*> initNodes_;
};

class VarDecl : public ExprNode {
  public:
    explicit VarDecl(QualType& T)
      :ExprNode(T, kVarDeclClass), pVarValue_(NULL) {}

    int setInitialier(const ExprResult& initialier);

    bool isArrayVar() const {
      return getQualType().isArrayElement();
    }
    const std::vector<ExprNode*>* getInitExprVec() {
      assert(isArrayVar() );
      return getInitVar()->getInitExprVec();
    }
    InitExprs* getInitVar() {
      return dynamic_cast<InitExprs*>(pVarValue_);
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
