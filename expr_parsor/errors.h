namespace mnb{
namespace expr{
  namespace error{
struct ErrorEntry{
  ErrorKind kind_;
  char* desc_;
}
enum ErrorKind{
  warn_shift_gt_typewidth,
};
}

const error::ErrorEntry ErrorKindDesc[] ={
  {error::warn_shift_negative, "shift count is negative"},
  {error::warn_shift_gt_typewidth,"shift count >= width of type"},
  {error::warn_shift_result_gt_typewidth, "left shift result bits is %0, but %2 has only %3 bits"},
  {error::warn_array_index_exceeds_bounds,"array index of '%0' indexes past the end of an array (that contains %1 element %2)"},
  {error::warn_array_index_precedes_bounds,"array index of '%0' indexes before the beginning of the array"},
  {error::err_typecheck_invalid_operands,"invalid operands to binary expression (%0 and %1)"},
  {error::err_typecheck_unary_expr, "invalid argument type %0 to unary expression"},
  {error::err_typecheck_subscript_not_integer, "array subscript is not an integer"},
  {error::err_typecheck_subscript_value,"subscripted value is not an array, pointer, or vector"},
};
class ErrorReport{
  public:
    explicit ErrorReport(int32_t lineNo);
      line_(lineNo){}
    ~ErrorReport();
  ErrorBuilder diagnose(int errorCode, int startcolumn);
  //StoreDiagnose
  //ErrorReport diagnose
  //ErrorReport& diagnose create a errorreport in ErrorReport::diagnose legal?
  private:
  int32_t line_;
  std::vector<ErrorMsg> errors_;
    struct ErrorEntry{
      int column_;
      std::string desc_;
    };
};

class ErrorBuilder{
  public:
    explicit ErrorBuilder(int32_t lineNo, int32_t columnNo, int32_t errorCode)
      :line_(lineNo)
       ,column_(columnNo)
       ,errorId_(errorCode)
       ,argumentNum_(0){;}
    ~ErrorBuilder();
    void addString(const std::string& argu);

  private:
    std::string formatErrorMessage(int errorIndex);
    void emit();

    int32_t line_;
    int32_t column_;
    int32_t errorId_;
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
