#include "ast.h"
namespace mnb{
namespace expr{
FunctionDecl* FunctionDecl::createBuiltinDecl(const BuiltinInfo* pbi){
  FunctionDecl* pFD = NULL;
  switch(pbi->builtinId_){
    case kBISIN:
      pFD = static_cast<FunctionDecl*>(new FDSin() );
      break;
    case kBICOS:
      pFD = static_cast<FunctionDecl*>(new FDCos() );
      break;
    case kBITG:
      pFD = static_cast<FunctionDecl*>(new FDTg() );
      break;
    case kBICTG:
      pFD = static_cast<FunctionDecl*>(new FDCtg() );
      break;
    case kBILOG:
      pFD = static_cast<FunctionDecl*>(new FDLog() );
      break;
    case kBIPOW:
      pFD = static_cast<FunctionDecl*>(new FDPow() );
      break;
    case kBIMAX:
      pFD = static_cast<FunctionDecl*>(new FDMax() );
      break;
    case kBIMIN:
      pFD = static_cast<FunctionDecl*>(new FDMin() );
      break;
    case kBIABS:
      pFD = static_cast<FunctionDecl*>(new FDAbs() );
      break;
    case kBISQRT:
      pFD = static_cast<FunctionDecl*>(new FDSqrt() );
      break;
    default:
      return NULL;
  }
  if (!initProtoTypes(pbi, pFD) )
    return NULL;
}

bool FunctionDecl::initProtoTypes(const BuiltinInfo* pbi, const FunctionDecl* pFuncDecl){
  if (!pFuncDecl)
    return false;
  const string& tmpTypestr = pbi->Type_;
  if(tmpTypestr.begin() == tmpTypestr.end() )
    return false;
  string::const_iterator scit = tmpTypestr.begin();
  pFuncDecl->setCallResultType(createTypeFromChar(*scit++) );
  for(; scit != tmpTypestr.end(); ++scit)
    pFuncDecl->pushArgumentType(createTypeFromChar(*scit));
  return true;
}

QualType FunctionDecl::createTypeFromChar(const char* typechar){
  //const struct TypeCharSt = {{"r", "REAL"}, {"d", "DINT"} };
  switch(typechar){
    case 'r':
      const IdentifierInfo* pII = symbol_table_.lookupIdentifier("REAL");
      const DeclType* pdt = symbol_table_.typeOf(pII.tag_);
      return QualType(pdt);
    case 'd':
      const IdentifierInfo* pII = symbol_table_.lookupIdentifier("DINT");
      const DeclType* pdt = symbol_table_.typeOf(pII.tag_);
      return QualType(pdt);
    default:
      return QualType();
  }
}
