namespace mnb{
namespace expr{
class Declarator{
  public:
    Declarator(QualType T, IdentifierInfo* pII)
      : DeclType_(T){}
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
    QualType getDeclType(){
      return DeclType_;
    }
    DeclaratorChunk& getChunckObject(int idx){
      if (0 < idx && idx < DeclInfo_.size() )
        return DeclInfo_[idx];
    };

  private:
    QualType DeclType_;
    std::vector<DeclaratorChunk> DeclInfo_;
};

class Sematic{
  public:
    Sematic();
    ~Sematic();
    
    VarDecl analyzeDeclartor(const Declarator& D);
    QualType getTypeInfoBy(const Declarator& D);
    QualType buildArrayType(QualType T, ExprNode* arraySize);
    ExprResult analyzeInitExprs(const std::vector<ExprNode>& initList);
};
}
}
