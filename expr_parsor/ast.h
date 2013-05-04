class Node{
};
class DeclType{
  public:
    DeclType(bool signd,uint32_t width,
            TyKinds ty, const string& literal)
      :signedness_(signd)
      ,typeWidth_(width)
      ,typeKind_(ty)
      ,typeLiteral_(literal){}
    enum TyKinds{
      kVoidTy,  kBoolTy,
      kIntTy,   kFloatTy,
    }
    ExprResult createDefaultInit();
//    false 0 0.0
  private:
    bool signedness_;
    uint32_t typeWidth_;
    TyKinds typeKind_;
    string typeLiteral_;
};
class QualType{
  public:
    bool isIntegralPromotion(const QualType& toType){
      // An rvalue of type char, signed char, unsigned char, short int, or
      // unsigned short int can be converted to an rvalue of type int if
      // int can represent all the values of the source type; otherwise,
      // the source rvalue can be converted to an rvalue of type unsigned
      // int (C++ 4.5p1).
      if (getKind() == kIntTy && To->getKind() == kIntTy)
        if (// We can promote any signed, promotable integer type to an int
            //(isSignedIntegerType() ||
             // We can promote any unsigned integer type whose size is
             // less than int to an int.
             (//!isSignedIntegerType() &&
              getTypeWidth() < toType.getTypeWidth())) {
          return true;
        }

        //return To.getKind() == kIntTy && !To->isSignedIntegerType();
      }
    }
    bool isArithmeticType(){
      return isIntegerType() || isRealFloatingType();
    }
class ExprNode{
  public:
    ExprNode(Token* pTok, QualType T)
      :pToken_(pTok)
       ,type_(T){}
    ExprResult Eval(){
    }
    QualType getQualType(){ return type_;}
  private:
    Token* pToken_;
    //tok::TokenTag tag_;
    Type type_;
};
class ConstArrayType{
  public:
    ConstArrayType(QualType element, int32_t size);
    ~ConstArrayType();
  private:
    QualType elemBase_;
    int32_t arraySize_;
};
class VarDecl : public ExprNode{
  VarDecl(Token* pTok, QualType T)
    : ExprNode(pTok, T){}
};
class BuiltinCallId : public ExprNode{
  public:
  private:
    FunctionDecl fnDecl_;
};
class BuiltinCallExpr : public ExprNode{
  public:

  private:
    Expr* lhsFn_;
    ExprVector arguments_;
    QualType resultTy_;
};
class Arith: public ExprNode{
  public:
    Arith(Token* pTok, ExprNode x1, ExprNode x2)
      : ExprNode(pTok, null_type){
        left_expr_ = x1;
        right_expr_ = x2;
        type_ = Type.max(x1.type_, x2.type_);
        if (type_ == null_type) {
          error("");
        }
      }
  private:
    ExprNode left_expr_;
    ExprNode right_expr_;
};
class Unary : public ExprNode{
  public:
    Unary(Token* pTok, ExprNode x)
      : ExprNode(pTok, null_type){
        expr_ = x;
        type_ = max(int, x.type_);
        if (type_ == null_type) {
          error("");
        }
      }
  private:
    ExprNode expr_;
};
class Constant : public ExprNode{
  public:
    Constant(Token* pTok, Type p)
      : ExprNode(pTok, p){}
    Constant(int i)
      : ExprNode(new Num(i), int){}
  private:
    Constant* TRUE_;
    Constant* FALSE_;
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
