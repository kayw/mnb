#include <iostream>
#include <limits>
#include "igloo/igloo_alt.h"
#include "../parser.h"
using namespace igloo;
using namespace mnb::expr;

Describe(ParserUnitTest) {
  It(IntAdd) {
    Parser p;
    const std::string& expr = "1 + 2 + (-3) + 5%3 - 2 * 3 / 6 - 0";
    ExprResult addResult = p.ParseExpression(expr);
    resVaule = addResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true, 1) ) );
    std::cout << std::endl;
  }
  It(FloatMultiply) {
    Parser p;
    ExprResult floatResult = p.ParseExpression("1.34 * 2.01 / 3.5 + 2 - 4.01");
    resVaule = floatResult.evaluate();
    std::cout << std::numeric_limits<float>::epsilon() << std::endl;
    Assert::That(resVaule.isEqual(ExprValue(float(-1.2404571428571427) ) ) );//-1.2404573
    std::cout << std::endl;
  }
  It(BoolLogical) {
    Parser p;
    ExprResult boolResult = p.ParseExpression("BOOL a = TRUE");
    boolResult = p.ParseExpression("BOOL b = FALSE");
    boolResult = p.ParseExpression("a && TRUE || OFF && ~b ");
    resVaule = boolResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true) ) );
    std::cout << std::endl;
  }
  It(IntBitwise) { 
    Parser p;
    ExprResult bitwiseResult = p.ParseExpression("USINT usi = 2");
    bitwiseResult = p.ParseExpression("DINT di = 3");
    bitwiseResult = p.ParseExpression("usi & di | 4 ^ 5");
    resVaule = bitwiseResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true, 3) ) );
    std::cout << std::endl;
  }
  It(ArrayParse) { 
    Parser p;
    ExprResult bitwiseResult = p.ParseExpression("USINT arr[] = { 1, 2, 3, 4, 7, 20}");
    bitwiseResult = p.ParseExpression("arr[3] >> 2");
    resVaule = bitwiseResult.evaluate();
    //constant(2)'s type is INT so the final result type promoted to signed
    Assert::That(resVaule.isEqual(ExprValue(/*false*/true, 1) ) );

    // warned about the bits limit, but still go on for result evaluatation
    bitwiseResult = p.ParseExpression("arr[2] << arr[4] ");
    resVaule = bitwiseResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(false, 384) ) );
  }
  It(SinCosParse) {
    Parser p;
    ExprResult sincosResult = p.ParseExpression("REAL real = 100000.0789");
    //assign different type in declaration specifier isn't allowed
    sincosResult = p.ParseExpression("UDINT udi = real");
    Assert::That(sincosResult.isInvalid() );
    sincosResult = p.ParseExpression("UDINT udi = 200");
    sincosResult = p.ParseExpression("SIN(real)*udi/COS(190) - SQRT(5) + LOG(3)*POW(2,2)");
    resVaule = sincosResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(float(-125.89104353331878) ) ) );//input bit length larger float error bigger todo
  }
  It(TwoDArrayParse) {
    Parser p;
    ExprResult dim2Result = p.ParseExpression("SINT si2a[][] = {{1,2,4}, {4,5,6}}");
    //FIXME:
    //array initialization {0}*n support
    //check array declaration subscript larger than initializer's number : VarDecl::setInitialier
    //figure out & refractor ast node type & success evaluate value
    dim2Result = p.ParseExpression("UDINT udi = si2a[0][2] * 10000 + 20");
    dim2Result = p.ParseExpression("udi");
    resVaule = dim2Result.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(false, 40020) ) );
  }
  ExprValue resVaule;
};
