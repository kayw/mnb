#include "builtinfn.h"
#include "symbols.h"
namespace mnb{
namespace expr{
FunctionDecl* FunctionDecl::createBuiltinDecl(const BuiltinInfo& bi, IdentifierTable& table){
  FunctionDecl* pFD = NULL;
  switch(bi.builtinID_){
    case BuiltinInfo::kBISIN:
      pFD = static_cast<FunctionDecl*>(new FDSin() );
      break;
    case BuiltinInfo::kBICOS:
      pFD = static_cast<FunctionDecl*>(new FDCos() );
      break;
    case BuiltinInfo::kBITG:
      pFD = static_cast<FunctionDecl*>(new FDTg() );
      break;
    case BuiltinInfo::kBICTG:
      pFD = static_cast<FunctionDecl*>(new FDCtg() );
      break;
    case BuiltinInfo::kBILOG:
      pFD = static_cast<FunctionDecl*>(new FDLog() );
      break;
    case BuiltinInfo::kBIPOW:
      pFD = static_cast<FunctionDecl*>(new FDPow() );
      break;
    case BuiltinInfo::kBIMAX:
      pFD = static_cast<FunctionDecl*>(new FDMax() );
      break;
    case BuiltinInfo::kBIMIN:
      pFD = static_cast<FunctionDecl*>(new FDMin() );
      break;
    case BuiltinInfo::kBIABS:
      pFD = static_cast<FunctionDecl*>(new FDAbs() );
      break;
    case BuiltinInfo::kBISQRT:
      pFD = static_cast<FunctionDecl*>(new FDSqrt() );
      break;
    default:
      break;
  }
  if (!initProtoTypes(bi, pFD, table) )
    SAFE_DELETE(pFD);
  return pFD;
}

bool FunctionDecl::initProtoTypes(const BuiltinInfo& bi, FunctionDecl* pFuncDecl, IdentifierTable& table){
  if (!pFuncDecl)
    return false;
  const MString& tmpTypestr = bi.Type_;
  if(tmpTypestr.begin() == tmpTypestr.end() )
    return false;
  MString::const_iterator scit = tmpTypestr.begin();
  pFuncDecl->setCallResultType(createTypeFromChar(*scit++, table) );
  for(; scit != tmpTypestr.end(); ++scit)
    pFuncDecl->pushArgumentType(createTypeFromChar(*scit, table) );//todo argument positive
  return true;
}

QualType FunctionDecl::createTypeFromChar(const char typechar, IdentifierTable& table){
  //const struct TypeCharSt = {{"r", "REAL"}, {"d", "DINT"} };
  const IdentifierInfo* pII = NULL;
  switch(typechar){
    case 'r':
      pII = table.lookupIdentifier("REAL");
      break;

    case 'd':
      pII = table.lookupIdentifier("DINT");
      break;

    default:
      break;
  }
  DeclType* pdt = pII ? table.typeOf(pII->tag_) : NULL;
  return QualType(static_cast<Type*>(pdt) );
}

}
}
