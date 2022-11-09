
typedef enum TokenType TokenType;
enum TokenType {
    TOKEN_INVALID,
    
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    
    TOKEN_PARENTHESIS_OPEN,
    TOKEN_PARENTHESIS_CLOSE,
    
    TOKEN_SEMICOLON,
    
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_RETURN,
    
    TOKEN_IDENTIFIER,
    TOKEN_LITERAL_INT,
    
    // Note(abiab): Operators
    TOKEN_EXCLAM,
    TOKEN_TILDE,
    
    TOKEN_MINUS,
    
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_SLASH_FORWARD,
    TOKEN_PERCENT,
    
    TOKEN_EQUALS,
    TOKEN_NOT_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_LESS_THAN,
    TOKEN_LESS_OR_EQUAL,
    TOKEN_GREATER_THAN,
    TOKEN_GREATER_OR_EQUAL,
    
    TOKEN_ASSIGN,
};

typedef struct Token Token;
struct Token {
    TokenType type;
    char * string;
    int string_length;
    
    // TODO(Abi): Decide how to handle this for further values
    unsigned int value;
};

typedef struct Tokeniser Tokeniser;
struct Tokeniser {
    char * buffer;
    int line_number;
    const char * file_name;
};

void PrintToken(Token token) {
    switch(token.type) {
        case (TOKEN_INVALID): {
            printf("TOKEN: INVALID (%i)\n", token.type);
        } break;
        
        case (TOKEN_BRACE_OPEN): {
            printf("TOKEN: '{' (%i)\n", token.type);
        } break;
        
        case (TOKEN_BRACE_CLOSE): {
            printf("TOKEN: '}' (%i)\n", token.type);
        } break;
        
        case (TOKEN_PARENTHESIS_OPEN): {
            printf("TOKEN: '(' (%i)\n", token.type); 
        } break;
        
        case (TOKEN_PARENTHESIS_CLOSE): {
            printf("TOKEN: ')' (%i)\n", token.type);
        } break;
        
        case (TOKEN_SEMICOLON): {
            printf("TOKEN: ';' (%i)\n", token.type);
        } break;
        
        case (TOKEN_KEYWORD_INT): {
            printf("TOKEN: KEYWORD: INT (%i)\n", token.type);
        } break;
        
        case (TOKEN_KEYWORD_RETURN): {
            printf("TOKEN: KEYWORD: RETURN (%i)\n", token.type);
        } break;
        
        case (TOKEN_LITERAL_INT): {
            printf("TOKEN: LITERAL: INT <%d> (%d)\n", token.value, token.type);
        } break;
        
        case (TOKEN_IDENTIFIER): {
            printf("TOKEN: IDENTIFIER <%.*s> (%d)\n", token.string_length, token.string, token.type);
        } break;
        
        case (TOKEN_EXCLAM): {
            printf("TOKEN: '!' (%d)\n", token.type);
        } break;
        
        case (TOKEN_MINUS): {
            printf("TOKEN: '-' (%d)\n", token.type);
        } break;
        
        case (TOKEN_TILDE): {
            printf("TOKEN: '~' (%d)\n", token.type);
        } break;
        
        case (TOKEN_PLUS): {
            printf("TOKEN: '+' (%d)\n", token.type);
        } break;
        
        case (TOKEN_STAR): {
            printf("TOKEN: '*' (%d)\n", token.type);
        } break;
        
        case (TOKEN_SLASH_FORWARD): {
            printf("TOKEN: '/' (%d)\n", token.type);
        } break;
        
        case (TOKEN_PERCENT): {
            printf("TOKEN: '%%' (%d)\n", token.type);
        } break;
        
        case (TOKEN_EQUALS): {
            printf("TOKEN: '==' (%d)\n", token.type);
        } break;
        
        case (TOKEN_NOT_EQUAL): {
            printf("TOKEN: '!=' (%d)\n", token.type);
        } break;
        
        case (TOKEN_AND): {
            printf("TOKEN: '&&' (%d)\n", token.type);
        } break;
        
        case (TOKEN_OR): {
            printf("TOKEN: '||' (%d)\n", token.type);
        } break;
        
        case (TOKEN_LESS_THAN): {
            printf("TOKEN: '<' (%d)\n", token.type);
        } break;
        
        case (TOKEN_LESS_OR_EQUAL): {
            printf("TOKEN: '<=' (%d)\n", token.type);
        } break;
        
        case (TOKEN_GREATER_THAN): {
            printf("TOKEN: '>' (%d)\n", token.type);
        } break;
        
        case (TOKEN_GREATER_OR_EQUAL): {
            printf("TOKEN: '>=' (%d)\n", token.type);
        } break;
        
        case (TOKEN_ASSIGN): {
            printf("TOKEN: '=' (%d)\n", token.type);
        } break;
        
        default: {
            printf("[Error] token (%d) has not been added to print function\n", token.type);
        }
    }
}

Token PeekToken(char * string) {
    Token token = {0};
    
    char * cursor = string;
    while(*cursor != '\0') {
        switch (*cursor) {
            // Note(abi): skip white space
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            {
                cursor++;
            } break;
            
            // Note(abiab): starts with a digit
            case '0': 
            case '1': 
            case '2': 
            case '3': 
            case '4': 
            case '5': 
            case '6': 
            case '7': 
            case '8': 
            case '9': {
                token.type = TOKEN_LITERAL_INT;
                token.string = cursor;
                char * tail = cursor;
                
                unsigned int value = 0;
                while(CharIsDigit(*tail)) {
                    value *= 10;
                    value += *tail - '0';
                    tail++;
                }
                token.value = value;
                token.string_length = tail - cursor;
                cursor = tail;
                
                goto found_token;
            } break;
            
            // Note(abiab): Starts w a letter.
            case 'A': 
            case 'B': 
            case 'C': 
            case 'D': 
            case 'E': 
            case 'F': 
            case 'G': 
            case 'H': 
            case 'I': 
            case 'J': 
            case 'K': 
            case 'L': 
            case 'M': 
            case 'N': 
            case 'O': 
            case 'P': 
            case 'Q': 
            case 'R': 
            case 'S': 
            case 'T': 
            case 'U': 
            case 'V': 
            case 'W': 
            case 'X': 
            case 'Y': 
            case 'Z':
            case 'a': 
            case 'b': 
            case 'c': 
            case 'd': 
            case 'e': 
            case 'f': 
            case 'g': 
            case 'h': 
            case 'i': 
            case 'j': 
            case 'k': 
            case 'l': 
            case 'm': 
            case 'n': 
            case 'o': 
            case 'p': 
            case 'q': 
            case 'r': 
            case 's': 
            case 't': 
            case 'u': 
            case 'v': 
            case 'w': 
            case 'x': 
            case 'y': 
            case 'z': 
            {
                // TODO(abiab): check if its a keyword, if it isn't, we assume it's an identifier
                token.type = TOKEN_IDENTIFIER; // TEMP
                token.string = cursor;
                char * tail = cursor;
                tail++;
                while(CharIsLetter(*tail) || CharIsDigit(*tail) || *tail == '_') {
                    tail++;
                }
                
                token.string_length = tail - cursor;
                cursor = tail;
                
                if(token.string_length == 3 && StringCompareN(token.string, "int", 3))
                    token.type = TOKEN_KEYWORD_INT;
                else if(StringCompareN(token.string, "return", MAX(token.string_length, 6)))
                    token.type = TOKEN_KEYWORD_RETURN;
                
                goto found_token;
            } break;
            
            
            case '{': {
                token.type = TOKEN_BRACE_OPEN;
                cursor += 1;
                
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case '}': {
                token.type = TOKEN_BRACE_CLOSE;
                cursor += 1;
                
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case '(': {
                token.type = TOKEN_PARENTHESIS_OPEN;
                cursor += 1;
                
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case ')': {
                token.type = TOKEN_PARENTHESIS_CLOSE;
                cursor += 1;
                
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case ';': {
                token.type = TOKEN_SEMICOLON;
                cursor += 1;
                
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case '~': {
                token.type = TOKEN_TILDE;
                token.string = cursor;
                token.string_length = 1;
                
                cursor += 1;
                
                goto found_token;
            } break;
            
            
            case '!': {
                if(*(cursor+1) == '=') {
                    token.type = TOKEN_NOT_EQUAL;
                    token.string = cursor;
                    token.string_length = 2;
                }
                else {
                    token.type = TOKEN_EXCLAM;
                    token.string = cursor;
                    token.string_length = 1;
                }
                
                goto found_token;
            } break;
            
            case '-': {
                token.type = TOKEN_MINUS;
                token.string = cursor;
                token.string_length = 1;
                
                cursor += 1;
                
                goto found_token;
            } break;
            
            case '+': {
                token.type = TOKEN_PLUS;
                
                token.string = cursor;
                token.string_length = 1;
                
                cursor += 1;
                goto found_token;
            } break;
            
            case '/': {
                token.type = TOKEN_SLASH_FORWARD;
                
                token.string = cursor;
                token.string_length = 1;
                
                cursor += 1;
                goto found_token;
            } break;
            
            case '%': {
                token.type = TOKEN_PERCENT;
                
                token.string = cursor;
                token.string_length = 1;
                
                cursor += 1;
                goto found_token;
            } break;
            
            case '*': {
                token.type = TOKEN_STAR;
                
                token.string = cursor;
                token.string_length = 1;
                
                goto found_token;
            } break;
            
            case '=': {
                if(*(cursor+1) == '=') {
                    token.type = TOKEN_EQUALS;
                    token.string = cursor;
                    token.string_length = 2;
                } 
                else {
                    token.type = TOKEN_ASSIGN;
                    token.string = cursor;
                    token.string_length = 1;
                }
                
                goto found_token;
            } break;
            
            case '&': {
                if(*(cursor+1) == '&') {
                    token.type = TOKEN_AND;
                    token.string = cursor;
                    token.string_length = 2;
                }
                
                goto found_token;
            } break;
            
            case '|': {
                if(*(cursor+1) == '|') {
                    token.type = TOKEN_OR;
                    token.string = cursor;
                    token.string_length = 2;
                }
                
                goto found_token;
            } break;
            
            case '<': {
                if(*(cursor+1) == '=') {
                    token.type = TOKEN_LESS_OR_EQUAL;
                    token.string = cursor;
                    token.string_length = 2;
                } else {
                    token.type = TOKEN_LESS_THAN;
                    token.string = cursor;
                    token.string_length = 1;
                }
                
                goto found_token;
            } break;
            
            case '>': {
                if(*(cursor+1) == '=') {
                    token.type = TOKEN_GREATER_OR_EQUAL;
                    token.string = cursor;
                    token.string_length = 2;
                } else {
                    token.type = TOKEN_GREATER_THAN;
                    token.string = cursor;
                    token.string_length = 1;
                }
                
                goto found_token;
            } break;
        }
    }
    
    
    found_token:;
    
    return token;
}

Token GetNextTokenAndAdvance(Tokeniser * tokeniser) {
    while(CharIsWhiteSpace(*tokeniser->buffer)) {
        if(*tokeniser->buffer == '\n') tokeniser->line_number++;
        
        tokeniser->buffer++;
    }
    Token token = PeekToken(tokeniser->buffer);
    tokeniser->buffer += token.string_length;
    return token;
}