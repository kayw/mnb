#ifndef BUILTINFN_H_76F7A7EE_D2B7_42B0_B825_142842091958
#define BUILTINFN_H_76F7A7EE_D2B7_42B0_B825_142842091958

#include "ast.h"
namespace mnb{
namespace expr{

struct BuiltinInfo;
class IdentifierTable;
class FunctionDecl{
  public:
    FunctionDecl()
      : resultTy_(NULL){}
    virtual ~FunctionDecl(){} 
    QualType getArgType(const int idx) const { return argumentTypes_[idx]; }
    int getProtoNumArgs() const { return argumentTypes_.size(); }
    QualType getCallResultType() const { return resultTy_; } 
    void setCallResultType(const QualType& type) { resultTy_ = type;}
    void pushArgumentType(const QualType& type)  { argumentTypes_.push_back(type); }
    virtual ExprValue operator()(ExprVector& arguments) = 0;
    static FunctionDecl* createBuiltinDecl(const BuiltinInfo& bi, IdentifierTable& table);
    static bool initProtoTypes(const BuiltinInfo& bi, FunctionDecl* pFuncDecl, IdentifierTable& table);
    static QualType createTypeFromChar(const char typechar, IdentifierTable& table);

  private:
    QualType resultTy_;
    std::vector<QualType> argumentTypes_;
};

class FDSin : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = sin(argu1.floatVal);
      return res;
    }
};

class FDCos : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = cos(argu1.floatVal);
      return res;
    }
};

class FDTg : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = tan(argu1.floatVal);
      return res;
    }
};

class FDCtg : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = 1.0/tan(argu1.floatVal);
      return res;
    }
};

class FDLog : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = log(argu1.floatVal);
      return res;
    }
};

class FDPow : public FunctionDecl {
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.intVal.uintValue = pow(argu1.intVal.uintValue, argu2.intVal.uintValue);
      res.intVal.isSigned = true;
      return res;
    }
};

class FDMax : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.intVal.uintValue = max_t(argu1.intVal.uintValue, argu2.intVal.uintValue);
      res.intVal.isSigned = true;
      return res;
    }
};

class FDMin : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.intVal.uintValue = min_t(argu1.intVal.uintValue, argu2.intVal.uintValue);
      res.intVal.isSigned = true;
      return res;
    }
};

class FDAbs : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.intVal.uintValue = fabs(argu1.intVal.uintValue);
      res.intVal.isSigned = false;
      return res;
    }
};

class FDSqrt : public FunctionDecl{
  public:
    virtual ExprValue operator()(ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = sqrt(argu1.floatVal);
      return res;
    }
};

}
}

#endif /* BUILTINFN_H_76F7A7EE_D2B7_42B0_B825_142842091958 */

