#include "symbols.h"
namespace mnb{
namespace expr{

IdentifierTable::IdentifierTable() {
  addKeywords();
}

IdentifierTable::~IdentifierTable() {
  std::for_each(IdentifierInfoMap_.begin(), IdentifierInfoMap_.end(),
      EntryDeleter() );
  std::for_each(BuiltinCallMap_.begin(), BuiltinCallMap_.end(),
      EntryDeleter() );
  std::for_each(DeclTypeMap_.begin(), DeclTypeMap_.end(),
      EntryDeleter() );
}

void IdentifierTable::addKeywords() {
#define KEYWORD(ID, FLAGS) \
  addKeyword(MString(#ID), Token::kw_ ## ID);
#define OPERATOR_ALIAS(ID, TOK) \
  addKeyword(MString(#ID), Token::TOK);
#define TYPEKEYWORD(ID, SIGNED, BYTES, KIND) \
  addTypeKeyword(MString(#ID), Token::kw_ ## ID, SIGNED, BYTES, KIND);
#define BUILTIN(ID, TYPE) \
  initializeBuiltinIDs(MString(#ID),BuiltinInfo::kBI##ID, TYPE);
#include "tokentag.def"
}

void IdentifierTable::addKeyword(const MString& id, Token::TokenTag tag) {
  IdentifierInfo* pIdi = lookupIdentifier(id);
  pIdi->tag_ = tag;
}

IdentifierInfo* IdentifierTable::lookupIdentifier( const MString& id ) {
 // hashmap::const_iterator cit = IdentifierInfoMap_.find(id);//todo
  std::map<MString, IdentifierInfo*>::const_iterator cit = IdentifierInfoMap_.find(id);
  if (cit != IdentifierInfoMap_.end() ) {
   return IdentifierInfoMap_[id]; 
  }
  IdentifierInfo* pii = new IdentifierInfo;
  //IdentifierInfoMap_.insert(id, pii);
  IdentifierInfoMap_.insert(std::map<MString, IdentifierInfo*>::value_type(id, pii) );
  return pii;
}

void IdentifierTable::addTypeKeyword(const MString& id, Token::TokenTag tag,
                                    bool signedflag, uint32_t bytes, TyKinds kind) {
  if (DeclTypeMap_.find(tag) != DeclTypeMap_.end() )
    return;
  DeclType* dt = new DeclType(signedflag, bytes, kind, id);
  DeclTypeMap_[tag] = dt;
}

bool IdentifierTable::findType(const Token::TokenTag tag) const {
  return DeclTypeMap_.find(tag) != DeclTypeMap_.end();
}

DeclType* IdentifierTable::typeOf(const Token::TokenTag tag) {
  assert(DeclTypeMap_.find(tag) != DeclTypeMap_.end());
  return DeclTypeMap_[tag];
}

void IdentifierTable::initializeBuiltinIDs(const MString& id, const BuiltinInfo::ID builtinID, const MString typeset ) {
  IdentifierInfo* pIDi = lookupIdentifier(id);
  BuiltinInfo* pBinf = new BuiltinInfo;
  pBinf->builtinID_ = builtinID;
  pBinf->Type_ = typeset;
  BuiltinCallMap_[pIDi] = pBinf;
}

BuiltinInfo* IdentifierTable::findBuiltinIDInfo(const IdentifierInfo* pIdInfo) {
  std::map<const IdentifierInfo*, BuiltinInfo*>::const_iterator bcIter = BuiltinCallMap_.find(pIdInfo);
  return (bcIter != BuiltinCallMap_.end() ) ? bcIter->second : NULL;
}

}
}
