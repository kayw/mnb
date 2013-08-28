namespace mnb{
namespace expr{
enum OperatorKind{
  kBOMul; kBODiv; kBORem; kBOAdd;
  kBOSub; kBOShl; kBOShr; kBOLE;
  kBOLT; kBOGE; kBOGT; kBONE;
  kBOEQ; kBOBitAnd; kBOBitXor; kBOBitOr; 
  kBOLAnd; kBOLOr; kBOAssign; kBOComma;
  kUOPlus; kUOMinus; kUONot; kUOLNot; 
};

class Declarator{
  public:
    Declarator(QualType T, IdentifierInfo* pII)
      : DeclType_(T)
      ,pIdInfo_(pII){}
    ~Declarator();
    struct DeclaratorChunk{
      enum{
        kArrayType,
      }ChunkKind;
      struct ArrayTypeInfo{
        ExprNode* ElemSize;
      };
      ArrayTypeInfo Arr_;
    };
    DeclaratorChunk createArrayChunk(ExprNode* elemExpr){
      DeclaratorChunk chunk;
      chunk.ChunkKind = kArrayType;
      chunk.Arr_.ElemSize = elemExpr;
      return chunk;
    }
    void AddTypeInfo(const DeclaratorChunk& dc){
      DeclInfo_.push_back(dc);
    }
    int getTypeObjects(){
      return DeclInfo_.size();
    }
    QualType getDeclType() const {
      return DeclType_;
    }
    IdentifierInfo* getIdentifier() const {
      return pIdInfo_;
    }
    DeclaratorChunk& getChunckObject(int idx){
      if (0 < idx && idx < DeclInfo_.size() )
        return DeclInfo_[idx];
    };

  private:
    IdentifierInfo* pIdInfo_;
    QualType DeclType_;
    std::vector<DeclaratorChunk> DeclInfo_;
};

class Sematic{
  public:
    Sematic(Lexer& l);
    ~Sematic();
    
    VarDecl analyzeDeclartor(const Declarator& D);
    QualType getTypeInfoBy(const Declarator& D);
    QualType buildArrayType(QualType T, ExprNode* arraySize);
    ExprResult analyzeInitExprs(const std::vector<ExprNode>& initList);

  private:
    std::map<IdentifierInfo*, VarDecl*> declGroupMap_;
    void Diag(int32_t diagid){
      lex_.Diag(diagid);
    }
    Lexer& lex_;
};
}
}
