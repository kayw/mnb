#include <iostream>
#include "mmacros.h"
#include "parser.h"
#include "errors.h"
using namespace mnb::expr;

int main(){
  std::string expressionStr;
  int32_t i = 1;
  ExprResult finalExprResult(true);
  ErrorReport er;
  Parser pa(er);
  while(std::cin >> expressionStr){
    er.setLineNo(i++);
    finalExprResult = pa.ParseExpression(expressionStr);
  }
  ExprValue ev = finalExprResult.evaluate();
}
