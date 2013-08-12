#include<math.h>
namespace mnb{
namespace expr{

class FunctionDecl{
  public:
    int getProtoNumArgs() const { return argumentTypes_.size(); }
    QualType getCallResultType() const { return resultTy_; } 
    void setCallResultType(const QualType& type){ resultTy_ = type;}
    void pushArgumentType(const QualType& type){ argumentTypes_.push_back(type); }
    ExprValue operator(const ExprVector& arguments);
    static FunctionDecl* createBuiltinDecl(const BuiltinInfo& bi);
    static bool initProtoTypes(const BuiltinInfo& bi, const FunctionDecl* pFuncDecl);
    static QualType createTypeFromChar(const char* typechar);

  private:
    QualType resultTy_;
    std::vector<QualType> argumentTypes_;
    const IdentifierTable& symbol_table_;
};

class FDSin : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = sin(argu1.floatVal);
      return res;
    }
}

class FDCos : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = cos(argu1.floatVal);
      return res;
    }
}

class FDTg : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = tan(argu1.floatVal);
      return res;
    }
}

class FDCtg : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = 1.0/tan(argu1.floatVal);
      return res;
    }
}

class FDLog : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = log(argu1.floatVal);
      return res;
    }
}

class FDPow : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.floatVal = pow(argu1.floatVal, argu2.floatVal);
      return res;
    }
}

class FDMax : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.intVal.uintValue = max(argu1.intVal.uintValue, argu2.intVal.uintValue);
      res.intVal.isSigned = true;
      return res;
    }
}

class FDMin : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 2);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      ExprValue argu2 = arguments[1].evaluate();
      res.intVal.uintValue = min(argu1.intVal.uintValue, argu2.intVal.uintValue);
      res.intVal.isSigned = true;
      return res;
    }
}

class FDAbs : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kIntTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.intVal.uintValue = abs(argu1.intVal.uintValue);
      res.intVal.isSigned = false;
      return res;
    }
}

class FDSqrt : public FunctionDecl{
  public:
    ExprValue operator(const ExprVector& arguments){
      assert(arguments.size() == 1);
      ExprValue res;
      res.valueType = kFloatTy;
      ExprValue argu1 = arguments[0].evaluate();
      res.floatVal = sqrt(argu1.floatVal);
      return res;
    }
}
