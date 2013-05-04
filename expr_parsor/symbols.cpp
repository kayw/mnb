#include "symbols.h"
namespace mnb{
namespace expr{
IdentifierTable::IdentifierTable(){
  addKeywords();
}
IdentifierTable::~IdentifierTable(){
  for_each(IdentifierInfoMap_.begin().second, IdentifierInfoMap_.end().second,
      IdentifierEntryDeleter() );
}
void IdentifierTable::addKeywords(){
#define KEYWORD(ID, FLAGS) \
  addKeyword(string(#ID), Token::kw_ ## ID);
#define OPERATOR_ALIAS(ID, TOK) \
  addKeyword(string(#ID), Token::kw_ ## TOK);
#define TYPEKEYWORD(ID, SIGNED, BYTES, KIND) \
  addTypeKeyword(string(#ID), Token::kw_ ## ID, SIGNED, BYTES, KIND);
#define BUILTIN(ID, TYPE) \
  initializeBuiltinIDs(string(#ID), TYPE);
#include "tokentag.def"
}

void IdentifierTable::addKeyword(const string& id, Token::TokenTag tag){
  IdentifierInfo* pIdi = lookupIdentifier(id);
  pIdi->tag_ = tag;
}
IdentifierInfo* IdentifierTable::lookupIdentifier( const string& id ){
  hashmap::const_iterator cit = IdentifierInfoMap_.find(id);
  if (cit != IdentifierInfoMap_.end() ) {
   return IdentifierInfoMap_[id]; 
  }
  IdentifierInfo* pii = new IdentifierInfo;
  IdentifierInfoMap_.insert(id, pii);
  return pii;
}
void IdentifierTable::addTypeKeyword(const string& id, Token::tokentag tag,
                                    bool signedflag, uint32_t bytes,DeclType::TyKinds kind){
  if (DeclTypeMap_.find(tag) != DeclTypeMap_.end() )
    return;
  DeclType* dt = new DeclType(signedflag, bytes, kind, id);
  DeclTypeMap_[tag] = dt;
}
bool IdentifierTable::findType(const Token::TokenTag tag){
  if (DeclTypeMap_.find(tag) != DeclTypeMap_.end() )
    return true;
  else
    return false;
}
DeclType* IdentifierTable::typeOf(const Token::TokenTag tag){
  return DeclTypeMap_[tag];
}

void IdentifierTable::initializeBuiltinIDs( const string& id, const string typeset )
{
  static int builtinNum = 0;
  IdentifierInfo* pIDi = lookupIdentifier(id);
  BuiltinInfo* pBinf = new BuiltinInfo;
  pBinf->builtinID_ = ++builtinNum;
  pBinf->Type_ = typeset;
  BuiltinCallMap_[pIDi] = pBinf;
}

}
}
