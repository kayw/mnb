#ifndef ERRORS_H_6F511C8D_26FD_4F4C_A187_247B2D68C4FE
#define ERRORS_H_6F511C8D_26FD_4F4C_A187_247B2D68C4FE

#include <algorithm>
#include "mmacros.h"
#include "mstring.h"
namespace mnb{
namespace expr{
namespace diag{

enum ErrorKind{
  warn_shift_negative = 0,
  warn_shift_gt_typewidth,
  warn_shift_result_gt_typewidth,
  warn_array_index_exceeds_bounds,
  warn_array_index_precedes_bounds,
  warn_division_by_zero,
  warn_remainder_by_zero,
  err_typecheck_invalid_operands,
  err_typecheck_unary_expr,
  err_typecheck_subscript_not_integer,
  err_typecheck_subscript_value,
  err_expected_semi_or_equal_declaration,
  err_expect_identifer,
  err_expected_expression,
  err_braceinitialiazer_is_initexpr,
  err_illegal_float_dot_part,
  err_invalid_array_element_size,
  err_function_argument_num_unfit,
  err_no_argument_conv,
  err_expected_token,
  diag_num
};

}

static const char digits[] = "9876543210123456789";
static const char* zero = digits + 9;
const uint32_t kMaxNumericSize = 250U;
// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
size_t formatInteger(char buf[], T value)
{
  T i = value;
  char* p = buf;

  do {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0) {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

const uint32_t kMaxArguments = 10U;
class ErrorReport;
class ErrorBuilder{
  public:
    explicit ErrorBuilder(ErrorReport* pReport)
      : pReporter_(pReport) {}
    ~ErrorBuilder(); 
    //copied
    ErrorBuilder(const ErrorBuilder& rhs){
      pReporter_ = rhs.pReporter_;
      rhs.pReporter_ = NULL;
    }
    void addString(const MString& argu);
    ErrorBuilder& operator<<(const MString& s){
      addString(s);
      return *this;
    }
    ErrorBuilder& operator<<(const int32_t& i){
      char intStr[kMaxNumericSize] = {0};
      int len = formatInteger(intStr, i);
      addString(MString(intStr, len) );
      return *this;
    }


  private:
    mutable ErrorReport* pReporter_;
    void operator=(const ErrorBuilder& rhs); 
};

class ErrorReport{
  public:
    ErrorReport()
      :line_(0), column_(0), diagId_(0)
       ,argumentNum_(0) {}
    explicit ErrorReport(int32_t lineNo)
      : line_(lineNo){}
    ~ErrorReport() { }
    ErrorBuilder diagnose(int diagCode, int startColumn);
    void setLineNo(int32_t lineNo) { line_ = lineNo; }
    void addString(const MString& argu);
    void emit();
  //StoreDiagnose
  //ErrorReport diagnose
  //ErrorReport& diagnose create a diagreport in ErrorReport::diagnose legal?
  private:
    MString formatErrorMessage(int diagIndex);
    int32_t line_;
    int32_t column_;
    int32_t diagId_;
    MString arguments_[kMaxArguments];
    int32_t argumentNum_;
};



//inline ErrorBuilder& operator<<(ErrorBuilder& eb, const QualType& t){
//  eb.addString(t.getName() );
//  return eb;
//}

//inline ErrorBuilder& operator<<(ErrorBuilder& eb, const uint32_t& i){
//  char intStr[kMaxNumericSize] = {0};
//  int len = formatInteger(intStr, i);
//  eb.addString(MString(intStr, len) );
//  return eb;
//}

}
}

#endif /* ERRORS_H_6F511C8D_26FD_4F4C_A187_247B2D68C4FE */

