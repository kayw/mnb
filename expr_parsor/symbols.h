#include "ast.h"
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
  string Type_;

  bool operator==(const BuiltinInfo &RHS) const {
    return builtinID_ == RHS.builtinID_ 
         &&!strcmp(Type_.c_str(), RHS.Type_.c_str() );
  }
  bool operator!=(const Info &RHS) const { return !(*this == RHS); }
};
class IdentifierInfo{
  public:
    IdentifierInfo()
      tag_(tok::identifer)
      {}
    tok::TokenTag tag_;
  private:
    DISALLOW_COPY_AND_ASSIGN(IdentifierInfo);    
};
class IdentifierTable{
  public:
    IdentifierTable(){}
    void addKeywords();
    IdentifierInfo* lookupIdentifier(const string& id);
  private:
    struct IdentifierEntryDeleter{
      void operator()(IdentifierInfo* pii){
        delete pii;
      }
    };
    void addKeyword(const mnb::string& id_name, Token::TokenTag kind);
    void initializeBuiltinIDs(const string& id, const string typeset);
    mnb::hashmap<mnb::string, IdentifierInfo*> IdentifierInfoMap_;
    std::map<IdentifierInfo*, BuiltinInfo*> BuiltinCallMap_;
    std::map<Token::TokenTag, DeclType*> DeclTypeMap_;
};
}
}
