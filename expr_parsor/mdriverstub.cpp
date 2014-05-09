#include <iostream>
#include "mmacros.h"
#include "parser.h"
#include "errors.h"
using namespace mnb::expr;

int main(){
  std::string expressionStr;
  ExprResult finalExprResult(true);
  Parser pa;
  while(std::cin >> expressionStr){
    finalExprResult = pa.ParseExpression(expressionStr);
  }
  ExprValue ev = finalExprResult.evaluate();
}
