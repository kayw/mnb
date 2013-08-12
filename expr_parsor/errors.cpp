namespace mnb{
namespace expr{

ErrorBuilder ErrorReport::diagnose(int errorCode, int startcolumn)
{
  return ErrorBuilder(line_, startcolumn, errorCode);
}

void ErrorBuilder::emit(){
  std::cout << "Line: " << line_ << " Column: " << column_ << " Error Info: ";
  std::string& msg_str = formatErrorMessage(errorId_);
  std::cout << msg_str << endline;
}

void ErrorBuilder::addString(const std::string& argu){
  arguments_[arguments_++] = argu;
}

std::string ErrorBuilder::formatErrorMessage(int32_t errorCode) {
  std::string outStr;
  std::string errorMsg = ErrorMsg[errorCode];
  std::string::const_iterator msgStr = errorMsg.begin();
  std::string::const_iterator msgEnd = errorMsg.end();
  while(msgStr != msgEnd ){
    if (*msgStr == '%') {
      ++msgStr;
      if (*msgStr == '%') {
        outStr += '%';
      }
      else{
        int32_t arguIdx = *msgStr++ - '0';
        assert(0 <= arguIdx && arguIdx < 10);
        outStr += arguments_[arguIdx];
      }
    }
    else{
      const char* strEnd = std::find(msgStr, msgEnd, '%');
      outStr.append(msgStr, strEnd);
      msgStr = strEnd;
    }
    ++msgStr;
  }
  return outStr;
}

}
}
  
