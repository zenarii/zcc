
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
    // todo, store the position and length of the token so that can see what it is called etc.
};

typedef struct Tokeniser Tokeniser;

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
            printf("TOKEN: LITERAL: INT (TODO PRINT VALUE!!!!)\n");
        } break;
        
        case (TOKEN_IDENTIFIER): {
            printf("TOKEN: IDENTIFIER (%.*s)\n", token.string_length, token.string);
        }
    }
}

char * GetNextToken(char * string) {
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
            
            // Note(abiab): Starts w a letter.
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
                
                token.string_length = tail - cursor;
                cursor = tail;
                
                goto found_token;
            } break;
            
            
            case '{': {
                token.type = TOKEN_BRACE_OPEN;
                cursor += 1;
                
                goto found_token;
            } break;
            
            case '}': {
                token.type = TOKEN_BRACE_CLOSE;
                cursor += 1;
                
                goto found_token;
            } break;
            
            case '(': {
                token.type = TOKEN_PARENTHESIS_OPEN;
                cursor += 1;
                
                goto found_token;
            } break;
            
            case ')': {
                token.type = TOKEN_PARENTHESIS_CLOSE;
                cursor += 1;
                
                goto found_token;
            } break;
            
            case ';': {
                token.type = TOKEN_SEMICOLON;
                cursor += 1;
                
                goto found_token;
            } break;
        }
    }
    
    found_token:;
    PrintToken(token);
    
    return cursor;
}