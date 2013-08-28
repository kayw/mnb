namespace mnb{
namespace expr{
namespace diag{
struct ErrorEntry{
  ErrorKind kind_;
  char* desc_;
}
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
  err_expected_token
};
}

const diag::ErrorEntry ErrorMsg[] ={
  {diag::warn_shift_negative, "shift count is negative"},
  {diag::warn_shift_gt_typewidth,"shift count >= width of type"},
  {diag::warn_shift_result_gt_typewidth, "left shift result bits is %0, but %2 has only %3 bits"},
  {diag::warn_array_index_exceeds_bounds,"array index of '%0' indexes past the end of an array (that contains %1 element %2)"},
  {diag::warn_array_index_precedes_bounds,"array index of '%0' indexes before the beginning of the array"},
  {diag::err_typecheck_invalid_operands,"invalid operands to binary expression (%0 and %1)"},
  {diag::err_typecheck_unary_expr, "invalid argument type %0 to unary expression"},
  {diag::err_typecheck_subscript_not_integer, "array subscript is not an integer"},
  {diag::err_typecheck_subscript_value,"subscripted value is not an array"},
  {diag::err_expected_semi_or_equal_declaration,"expected ';' or '=' after declaration %1"},
  {diag::err_expect_identifer,"expected identifer after declaration %1"},
  {diag::err_expected_expression, "expected valid expression"},
  {diag::err_braceinitialiazer_is_initexpr, "Init expression can't be used as brace initialiazer"},
  {diag::err_illegal_float_dot_part, "expectd digit after %1's dot"},
  {diag::warn_division_by_zero, "division by zero is undefined"},
  {diag::warn_remainder_by_zero,"remainder by zero is undefined"},
  {diag::err_invalid_array_element_size,"array element size expression is null"},
  {diag::err_function_argument_num_unfit,"function parameter number isn't compatible with builtin function prototype"},
  {diag::err_no_argument_conv,"no available conversion for parameter %0 to prototype parameter %1"},
  {diag::err_expected_token, "expected token %0"}

};
class ErrorReport{
  public:
    explicit ErrorReport(int32_t lineNo);
      line_(lineNo){}
    ~ErrorReport();
  ErrorBuilder diagnose(int diagCode, int startcolumn);
  //StoreDiagnose
  //ErrorReport diagnose
  //ErrorReport& diagnose create a diagreport in ErrorReport::diagnose legal?
  private:
  int32_t line_;
  //std::vector<ErrorMsg> diags_;
    struct ErrorEntry{
      int column_;
      std::string desc_;
    };
};

class ErrorBuilder{
  public:
    explicit ErrorBuilder(int32_t lineNo, int32_t columnNo, int32_t diagCode)
      :line_(lineNo)
       ,column_(columnNo)
       ,diagId_(diagCode)
       ,argumentNum_(0){;}
    ~ErrorBuilder();
    void addString(const std::string& argu);

  private:
    std::string formatErrorMessage(int diagIndex);
    void emit();

    int32_t line_;
    int32_t column_;
    int32_t diagId_;
    std::string arguments_[kMaxArguments];
    int32_t argumentNum_;
};

inline ErrorBuilder& operator<<(const ErrorBuilder& eb, const std::string& s){
  eb.addString(s);
  return eb;
}
inline ErrorBuilder& operator<<(const ErrorBuilder& eb, const QualType& t){
  eb.addString(t.getName() );
  return eb;
}
inline ErrorBuilder& operator<<(const ErrorBuilder& eb, const int32_t& i){
  eb.addString(toString(i) );
  return eb;
}
}
}
