#ifndef SEMATICS_H_3345EACF_8B83_45D0_B5CA_B9AC68D64F37
#define SEMATICS_H_3345EACF_8B83_45D0_B5CA_B9AC68D64F37

#include "ast.h"
#include "lexer.h"
#include "error.h"
namespace mnb{
namespace expr{

class IdentifierInfo;
class Declarator{
  public:
    Declarator(QualType T, const IdentifierInfo* pII)
      :pIdInfo_(pII)
      ,DeclType_(T) {}
    ~Declarator() { pIdInfo_ = NULL; }
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
      chunk.ChunkKind = DeclaratorChunk::kArrayType;
      chunk.Arr_.ElemSize = elemExpr;
      return chunk;
    }
    void addTypeInfo(const DeclaratorChunk& dc){
      DeclInfo_.push_back(dc);
    }
    int getTypeObjects() const {
      return DeclInfo_.size();
    }
    QualType getDeclType() const {
      return DeclType_;
    }
    const IdentifierInfo* getIdentifier() const {
      return pIdInfo_;
    }
    const DeclaratorChunk& getChunckObject(int idx) const {
      assert(0 <= idx && idx < static_cast<int>(DeclInfo_.size() ) );
      return DeclInfo_[idx];
    };

  private:
    const IdentifierInfo* pIdInfo_;
    QualType DeclType_;
    std::vector<DeclaratorChunk> DeclInfo_;
};

class Sematic{
  public:
    Sematic()
      :plex_(NULL), psymbol_table_(NULL) {}
    ~Sematic() {
      plex_ = NULL;
      psymbol_table_ = NULL;
    }
    
    void setSemaDependency(Lexer* pL, IdentifierTable& table) { plex_ = pL; psymbol_table_ = &table; }
    
    VarDecl* analyzeDeclarator(const Declarator& D);
    ExprResult analyzeInitExprs(std::vector<ExprNode*>& initList);
    ExprResult analyzeIdentifier(const IdentifierInfo* pII);
    ExprResult analyzeConstantLiteral(Token* pToken);
    void actOnUninitializedDecl(VarDecl* decl);
    ExprResult analyzeBinaryOperator(const Token::TokenTag opTokKind, ExprResult& lhs, ExprResult& rhs);
    ExprResult analyzeArraySubscript(ExprResult base, ExprResult idx);
    ExprResult analyzeUnaryOperator(Token::TokenTag Kind, ExprNode* InputExpr);
    ExprResult analyzeBuiltinCallExpr(ExprResult& fnLhs, ExprVector& args);

  private:
    ExprResult Owned(ExprNode* E);

    QualType getTypeInfoBy(const Declarator& D);
    QualType buildArrayType(QualType T, ExprNode* arraySize);
    QualType checkMultiplyDivOperands(ExprResult &lex, ExprResult &rex, bool isDiv);
    QualType checkRemainderOperands(ExprResult& lex, ExprResult& rex);
    QualType checkAdditionSubtractOperands(ExprResult &lex, ExprResult &rex);
    QualType checkShiftOperands(ExprResult &lex, ExprResult &rex, unsigned Opc);
    QualType checkCompareOperands(ExprResult &lex, ExprResult &rex, bool isRelational);
    QualType checkBitwiseOperands(ExprResult &lex, ExprResult &rex);
    QualType checkLogicalOperands(ExprResult &lex, ExprResult &rex);
    QualType checkAssignmentOperands(ExprNode* LHS, ExprResult &RHS);
    int getIntegerTypeRank(const QualType& lhsTy, const QualType& rhsTy);
    QualType arithmeticConversion(ExprResult& lhsExpr, ExprResult& rhsExpr);
    ExprResult parameterConversion(const ExprResult& Arg, const QualType& protoType);
    ExprResult createCastExprByType(ExprNode *E, QualType Ty, CastKind Kind);
    QualType arithIntegerCast(ExprResult& lhsExpr, ExprResult& rhsExpr);
    CastKind implicitCast(QualType& fromType, const QualType& toType);
    QualType invalidOperands(ExprResult &lex, ExprResult &rex);
    void diagnoseBadShiftValues(ExprResult &lex, ExprResult &rex, unsigned Opc);
    void CheckArrayAccess(const ExprNode* expr);
    
    static OperatorKind getUnaryOpcode(const Token::TokenTag Kind);
    static OperatorKind getBinaryOpcode(const Token::TokenTag Kind);

    std::map<const IdentifierInfo*, VarDecl*> declGroupMap_;
    ErrorBuilder Diag(int32_t diagid){
      return plex_->Diag(diagid);
    }
    Lexer* plex_;
    IdentifierTable* psymbol_table_;
};

}
}

#endif /* SEMATICS_H_3345EACF_8B83_45D0_B5CA_B9AC68D64F37 */
