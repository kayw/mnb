#ifndef SYMBOLS_H_2E2A2113_946F_4356_A34B_299ACFA58FA6
#define SYMBOLS_H_2E2A2113_946F_4356_A34B_299ACFA58FA6

//#include "hashmap.h"
#include <map>
#include <algorithm>
#include "string.h"
#include "ast.h"
#include "token.h"
namespace mnb{
namespace expr{

struct BuiltinInfo {
  enum ID {
	  NotBuiltin  = 0,      // This is not a builtin function.
#define BUILTIN(ID, TYPE) kBI##ID,
#include "tokentag.def"
	  BuiltinNum
	};
  int builtinID_;
  MString Type_;

  bool operator==(const BuiltinInfo &RHS) const {
    return builtinID_ == RHS.builtinID_ 
         && !strcmp(Type_.c_str(), RHS.Type_.c_str() );
  }
  bool operator!=(const BuiltinInfo &RHS) const { return !(*this == RHS); }
};

class IdentifierTable{
  public:
    IdentifierTable();
    ~IdentifierTable();
    IdentifierInfo* lookupIdentifier(const MString& id);
    DeclType* typeOf(const Token::TokenTag tag);
    bool findType(const Token::TokenTag tag) const;
    BuiltinInfo* findBuiltinIDInfo(const IdentifierInfo* pIdInfo);
  private:
    struct EntryDeleter{
      template <class T>
      void operator()(const T& entry) const {
        delete entry.second;
      }
    };
    void addKeywords();
    void addKeyword(const MString& id_name, Token::TokenTag kind);
    void addTypeKeyword(const MString& id, Token::TokenTag tag,
                      bool signedflag, uint32_t bytes, TyKinds kind);
    void initializeBuiltinIDs(const MString& id, const BuiltinInfo::ID builtinID, const MString typeset);
    //mnb::hashmap<mnb::MString, IdentifierInfo*> IdentifierInfoMap_;
    
    std::map<MString, IdentifierInfo*> IdentifierInfoMap_;
    std::map<const IdentifierInfo*, BuiltinInfo*> BuiltinCallMap_;
    std::map<Token::TokenTag, DeclType*> DeclTypeMap_;
    DISALLOW_COPY_AND_ASSIGN(IdentifierTable);
};

}
}

#endif /* SYMBOLS_H_2E2A2113_946F_4356_A34B_299ACFA58FA6 */

