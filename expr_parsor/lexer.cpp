namespace mnb{
namespace expr{
void Lexer::scan(Token& tok){
  const char* curPtr = bufferPtr_;
  for (; curPtr == ' ' || curPtr == '\t'; ++curPtr)
    ;
  char peek = advanceChar(curPtr);
  Token::TokenTag tag = Token::unknown;
  switch(peek){
    case '0': case '1': case '2': case '3':case '4':
    case '5': case '6': case '7': case '8':case '9':
      return lexNumericToken(curPtr);
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
  case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
  case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
  case 'V': case 'W': case 'X': case 'Y': case 'Z':
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
  case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
  case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
  case 'v': case 'w': case 'x': case 'y': case 'z'://  case '_':
      return lexIdentifier(curPtr);
 // case '"':
 //     return lexStringLiteral(tok);
  case '[':
      tag = Token::l_square;
      break;
  case ']':
      tag = Token::r_square;
      break;
  case '(':
      tag = Token::l_paren;
      break;
  case ')':
      tag = Token::r_paren;
      break;
  case '~':
      tag = Token::tilde;
      break;
  case '*': //case L'×':
      tag = Token::star; 
      break;
  case '/':
      tag = Token::slash;
      break;
  case '%':
      tag = Token::percent;
      break;
  case '+':
      tag = Token::plus;
      break;
  case '-':
      tag = Token::minus;
      break;
  case '<':
      if (nextCharIs('<', curPtr) ) {
        tag = Token::lessless;
      }
      else if (nextCharIs('=', curPtr) ) {
        tag = Token::lessequal; 
      }
      else if (nextCharIs('>', curPtr) ) {
        tag = Token::lessgreater;
      }
      else {
        tag = Token::less;
      }
      break;
  case '>':
      if (nextCharIs('>', curPtr) ) {
        tag = Token::greatergreater;
      }
      else if (nextCharIs('=', curPtr) ) {
        tag = Token::greaterequal;
      }
      else{
        tag = Token::greater;
      }
      break;
  case '!':
      if (nextCharIs('=', curPtr) ) {
        tag = Token::exclaimequal;
      }
      else{
        tag = Token::unknown;
      }
      break;
  case '=':
      if (nextCharIs('=', curPtr) ) {
        tag = Token::equalequal;
      }
      else{
        tag = Token::equal;
      }
      break;
  case '&':
      if (nextCharIs('&', curPtr) ) {
        tag = Token::ampamp;
      }
      else{
        tag = Token::amp;
      }
      break;
  case '^':
      tag = Token::caret;
      break;
  case '|':
      if (nextCharIs('|', curPtr) ) {
        tag = Token::pipepipe;
      }
      else{
        tag = Token::pipe;
      }
      break;
  default:
      tag = Token::unknown;
      break;
  }
  bufferPtr_ = curPtr;
  return new Token(tag);
}

Token* Lexer::lexNumericToken(const char* curPtr){//todo: check over under flow hex octal diag
  char ch = 0;
  Integer v = 0;
  readNumeric(curPtr, ch, v);
  const char* dotPtr = curPtr;
  if (ch == '.') {
    curPtr = consumeNextChar(curPtr);
    if (!isdigit(getNextChar(curPtr) ) {
      errorReport.fill("no digits after float %d's dot", v);
    }
    readNumeric(curPtr, ch, v);
  }
  exponent_t realexpon = static_cast<exponent_t>(dotPtr - curPtr);
  if (ch == 'e' || ch == 'E') {
    curPtr = consumeNextChar(curPtr);
    realexpon += readExponent(curPtr);
  }
  bufferPtr_ = curPtr;
  if (dotPtr == curPtr) {
    return (Token*)new Num(v);
  }
  else{
    double f = v * pow(10, realexpon);
    return new Real(f);
  }
}

void readNumeric(const char* curPtr, char& peek, Integer& inital){
  char ch = getNextChar(curPtr);
  while(isdigit(ch) ){
    curPtr = consumeNextChar(curPtr);
    inital = inital*10 + ch - '0';
    ch = getNextChar(curPtr);
  }
  peek = ch;
}

exponent_t readExponent(const char* curPtr) {
  char ch = getNextChar(curPtr);
  int tooLargeExponent = 24000;
  bool isNegative = (ch == '-');
  if (ch == '-' || ch == '+') {
    consumeNextChar(curPtr);
  }
  ch = getNextChar(curPtr);
  exponent_t expon = 0;
  while(isdigit(ch) ){
    curPtr = consumeNextChar(curPtr);
    expon = expon*10 + ch - '0';
    if (expon > tooLargeExponent) {
      expon = tooLargeExponent;
      break;
    }
    ch = getNextChar(curPtr);
  }
  return (isNegative ? -expon : expon);
}

Token* lexIdentifier(const char* curPtr){
  unsigned char ch = *curPtr++;
  while(isalpha(ch) || ch == '_')
    ch = *curPtr++;
  --curPtr;
  string identifier_name(bufferPtr_, curPtr - bufferPtr_);//todo:stringref
  IdentiferInfo& ii = identifier_table_.lookupIdentifier(identifier_name);
  bufferPtr_ = curPtr;
  return (Token*)new Word(identifier_name, &ii);
}
}
}
