
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
    // todo store like, line info etc
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
            printf("TOKEN: LITERAL: INT (%d) \n", token.value);
        } break;
        
        case (TOKEN_IDENTIFIER): {
            printf("TOKEN: IDENTIFIER (%.*s)\n", token.string_length, token.string);
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
                while(CharIsLetter(*tail) || CharIsDigit(*tail)) {
                    tail++;
                }
                
                if(StringCompareN(token.string, "int", MAX(token.string_length, 3)))
                    token.type = TOKEN_KEYWORD_INT;
                else if(StringCompareN(token.string, "return", MAX(token.string_length, 6)))
                    token.type = TOKEN_KEYWORD_RETURN;
                
                token.string_length = tail - cursor;
                cursor = tail;
                
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
        }
    }
    
    
    found_token:;
    //PrintToken(token);
    
    return token;
}

Token GetNextTokenAndAdvance(Tokeniser * tokeniser) {
    while(CharIsWhiteSpace(*tokeniser->buffer)) {
        tokeniser->buffer++;
    }
    Token token = PeekToken(tokeniser->buffer);
    tokeniser->buffer += token.string_length;
    return token;
}