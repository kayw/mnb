#include <iostream>
#include <limits>
#include "igloo/igloo_alt.h"
#include "../parser.h"
using namespace igloo;
using namespace mnb::expr;

Describe(ParserUnitTest) {
  It(IntAdd) {
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    const std::string& expr = "1 + 2 + (-3) + 5%3 - 2 * 3 / 6 - 0";
    ExprResult addResult = p.ParseExpression(expr);
    resVaule = addResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true, 1) ) );
    std::cout << std::endl;
  }
  It(FloatMultiply) {
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    ExprResult floatResult = p.ParseExpression("1.34 * 2.01 / 3.5 + 2 - 4.01");
    resVaule = floatResult.evaluate();
    std::cout << std::numeric_limits<float>::epsilon() << std::endl;
    Assert::That(resVaule.isEqual(ExprValue(float(-1.2404571428571427) ) ) );//-1.2404573
    std::cout << std::endl;
  }
  It(BoolLogical) {
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    ExprResult boolResult = p.ParseExpression("BOOL a = TRUE");
    er.setLineNo(2);
    boolResult = p.ParseExpression("BOOL b = FALSE");
    er.setLineNo(3);
    boolResult = p.ParseExpression("a && TRUE || OFF && ~b ");
    resVaule = boolResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true) ) );
    std::cout << std::endl;
  }
  It(IntBitwise) { 
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    ExprResult bitwiseResult = p.ParseExpression("USINT usi = 2");
    er.setLineNo(2);
    bitwiseResult = p.ParseExpression("DINT di = 3");
    er.setLineNo(3);
    bitwiseResult = p.ParseExpression("usi & di | 4 ^ 5");
    resVaule = bitwiseResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(true, 3) ) );
    std::cout << std::endl;
  }
  It(ArrayParse) { 
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    ExprResult bitwiseResult = p.ParseExpression("USINT arr[] = { 1, 2, 3, 4, 7, 20}");
    er.setLineNo(2);
    bitwiseResult = p.ParseExpression("arr[3] >> 2");
    resVaule = bitwiseResult.evaluate();
    //constant(2)'s type is INT so the final result type promoted to signed
    Assert::That(resVaule.isEqual(ExprValue(/*false*/true, 1) ) );

    er.setLineNo(3);
    bitwiseResult = p.ParseExpression("arr[2] << arr[4] ");
    resVaule = bitwiseResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(false, 384) ) );//todo warn shift gt type debug

    //todo
    //er.setLineNo(4);
    //bitwiseResult = p.ParseExpression("arr[3] << arr[5]");
    //resVaule = bitwiseResult.evaluate();
    //Assert::That(true/*todo*/);
    //std::cout << std::endl;
    //l << r  l.validbit + r.len > typewidth
  }
  It(SinCosParse) {
    ErrorReport er;
    Parser p(er);
    er.setLineNo(1);
    ExprResult sincosResult = p.ParseExpression("REAL real = 100000.0789");
    er.setLineNo(2);
    //assign different type in declaration specifier isn't allowed
    sincosResult = p.ParseExpression("UDINT udi = real");
    Assert::That(sincosResult.isInvalid() );
    er.setLineNo(3);
    sincosResult = p.ParseExpression("UDINT udi = 200");
    er.setLineNo(4);
    sincosResult = p.ParseExpression("SIN(real)*udi/COS(190) - SQRT(5) + LOG(3)*POW(2,2)");
    resVaule = sincosResult.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(float(-125.89104353331878) ) ) );//input bit length larger float error bigger todo
  }
  It(TwoDArrayParse) {
    ErrorReport er;
    Parser p(er);
    ExprResult dim2Result = p.ParseExpression("SINT si2a[][] = {{1,2,4}, {4,5,6}}");
    //FIXME:
    //array initialization {0}*n support
    //check array declaration subscript larger than initializer's number : VarDecl::setInitialier
    er.setLineNo(2);
    dim2Result = p.ParseExpression("UDINT udi = si2a[0][2] * 10000 + 20");
    er.setLineNo(3);
    dim2Result = p.ParseExpression("udi");
    resVaule = dim2Result.evaluate();
    Assert::That(resVaule.isEqual(ExprValue(false, 40020) ) );
  }
  ExprValue resVaule;
};

int main(int argc, char *argv[])
{
  return TestRunner::RunAllTests(argc, argv);
}
