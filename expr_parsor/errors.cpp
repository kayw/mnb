#include <iostream>
#include "errors.h"

namespace mnb{
namespace expr{

static const char* ErrorMsg[] ={
  "shift count is negative", //warn_shift_negative
  "shift count >= width of type",                                                        //warn_shift_gt_typewidth,
  "left shift result bits is %0, but %2 has only %3 bits",                                //warn_shift_result_gt_typewidth,
  "array index of '%0' indexes past the end of an array (that contains %1 elements)",     //warn_array_index_exceeds_bounds,
  "array index of '%0' indexes before the beginning of the array",                        //warn_array_index_precedes_bounds,
  "division by zero is undefined",                                                        //warn_division_by_zero,
  "remainder by zero is undefined",                                                       //warn_remainder_by_zero,
  "invalid operands to binary expression (%0 and %1)",                                    //err_typecheck_invalid_operands,
  "invalid argument type %0 to unary expression",                                         //err_typecheck_unary_expr,
  "array subscript is not an integer",                                                    //err_typecheck_subscript_not_integer,
  "subscripted value is not an array",                                                    //err_typecheck_subscript_value,
  "Number %0 initialier don't have the qualified type",                                       //err_typecheck_initialier_different
  "Right-side assignment type %0 don't match the decl type %1",          //err_typecheck_mismatch_decl_type_assignment
  "expected ';' or '=' after declaration %1",                                             //err_expected_semi_or_equal_declaration,
  "expected identifer after declaration %1",                                              //err_expect_identifer,
  "expected valid expression",                                                            //err_expected_expression,
  "empty expression list between braces",                                                 //err_empty_brace_initialiazer
  "expectd digit after %1's dot",                                                         //err_illegal_float_dot_part,
  "array element size expression is null",                                                //err_invalid_array_element_size,
  "function parameter number isn't compatible with builtin function prototype",           //err_function_argument_num_unfit,
  "no available conversion for parameter %0 to prototype parameter %1",                   //err_no_argument_conv
  "expected token %0",                                                                    //err_expected_token,
};

ErrorBuilder ErrorReport::diagnose(int errorCode, int startColumn)
{
  column_ = startColumn;
  diagId_ = errorCode;
  argumentNum_ = 0;
  return ErrorBuilder(this);
}

void ErrorReport::emit(){
  std::cout << "Line: " << line_ << " Column: " << column_ << " Error Info: ";
  const MString& msg_str = formatErrorMessage(diagId_);
  std::cout << msg_str << std::endl;
}

void ErrorReport::addString(const MString& argu){
  arguments_[argumentNum_++] = argu;
}

MString ErrorReport::formatErrorMessage(int32_t errorCode) {
  MString outStr;
  MString errorMsg = ErrorMsg[errorCode];
  MString::const_iterator msgStr = errorMsg.begin();
  MString::const_iterator msgEnd = errorMsg.end();
  while(msgStr != msgEnd ){
    if (*msgStr == '%') {
      ++msgStr;
      if (*msgStr == '%') {
        outStr += '%';
        ++msgStr;
      }
      else{
        int32_t arguIdx = *msgStr++ - '0';
        assert(0 <= arguIdx && arguIdx < 10);
        outStr += arguments_[arguIdx];
      }
    }
    else{
      MString::const_iterator strEnd = std::find(msgStr, msgEnd, '%');
      outStr.append(msgStr, strEnd);//todo
      msgStr = strEnd;
    }
  }
  return outStr;
}


void ErrorBuilder::addString(const MString& argu){
  pReporter_->addString(argu);
}

ErrorBuilder::~ErrorBuilder(){
  if (pReporter_)
  {
    pReporter_->emit();
  }
}
  

}
}
  
