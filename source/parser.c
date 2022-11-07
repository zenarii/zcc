typedef enum Associativity Associativity;
enum Associativity {
    ASSOCIATE_INVALID,
    
    ASSOCIATE_LEFT,
};

typedef struct Parselet Parselet;
struct Parselet {
    TokenType token;
    int precedence;
    Associativity associativty;
};

static Parselet parselets[] = {
    {TOKEN_PLUS,          5, ASSOCIATE_LEFT},
    {TOKEN_MINUS,         5, ASSOCIATE_LEFT},
    {TOKEN_STAR,          7, ASSOCIATE_LEFT},
    {TOKEN_SLASH_FORWARD, 7, ASSOCIATE_LEFT},
};

Parselet ParseletLookUp(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:          return parselets[0];
        case TOKEN_MINUS:         return parselets[1];
        case TOKEN_STAR:          return parselets[2];
        case TOKEN_SLASH_FORWARD: return parselets[3];
    }
}

//
// ~AST
//

typedef enum AstNodeType AstNodeType;
enum AstNodeType {
    AST_NODE_INVALID,
    
    AST_NODE_PROGRAM,
    AST_NODE_FUNCTION,
    AST_NODE_STATEMENT,
    AST_NODE_EXPRESSION,
    AST_NODE_BINARY_OPERATOR,
    AST_NODE_UNARY_OPERATOR,
    AST_NODE_ATOM,
};

typedef enum OperatorType OperatorType;
enum OperatorType {
    OPERATOR_INVALID,
    
    OPERATOR_PLUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MODULO,
    OPERATOR_MINUS_BINARY,
    
    OPERATOR_MINUS_UNARY,
    OPERATOR_NEGATE,
    OPERATOR_BITWISE_COMP,
};

typedef struct AstNode AstNode;
struct AstNode {
    AstNodeType type;
    // TODO(abi): remove?
    int subtype;
    
    AstNode * child;
    // TODO(abi): for binary operators
    AstNode * left_child;
    AstNode * right_child;
    
    // for function declaration
    char * function_name;
    int function_name_length;
    
    // for integer literal
    int int_literal_value;
    
    // for unary operator
    char unary_operator_character; // TODO(abi): convert to operator type usage
    OperatorType operator_type;
};


// TODO(abi): for now use an array of ast nodes, later make the size editable.
static AstNode nodes[128];
static int node_count;
static int parse_failed;

void ParserFail(Tokeniser * tokeniser, const char * error_message, TokenType skip_to) {
    parse_failed = 1;
    printf("[Error] %s:%d; %s\n", 
           tokeniser->file_name, tokeniser->line_number, error_message);
    
    // Note(abiab): advance the tokens to the next ; or } as to find further errors, as
    //              well as not to give incorrect errors
    while(skip_to) {
        Token token = PeekToken(tokeniser->buffer);
        if(token.type == skip_to || token.type == TOKEN_INVALID) {
            break;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
}

int IsUnaryOperator(TokenType type) {
    return (type == TOKEN_MINUS) || (type == TOKEN_EXCLAM) || (type == TOKEN_TILDE);
}

int IsBinaryOperator(TokenType type) {
    return ((type == TOKEN_STAR) ||
            (type == TOKEN_SLASH_FORWARD)||
            (type == TOKEN_MINUS) ||
            (type == TOKEN_PLUS));
}

OperatorType BinaryOperatorType(TokenType token_type) {
    switch (token_type) {
        case TOKEN_STAR:          return OPERATOR_MULTIPLY;
        case TOKEN_PLUS:          return OPERATOR_PLUS;
        case TOKEN_MINUS:         return OPERATOR_MINUS_BINARY;
        case TOKEN_SLASH_FORWARD: return OPERATOR_DIVIDE;
    }
}

//
// ~Parsing Functions
//

AstNode * ParseExpression(Tokeniser * tokeniser, int min_precedence);
void PrettyPrintAST(AstNode * node, int depth);
AstNode * ParseAtom(Tokeniser * tokeniser) {
    AstNode * atom;
    
    if(PeekToken(tokeniser->buffer).type == TOKEN_PARENTHESIS_OPEN) {
        GetNextTokenAndAdvance(tokeniser);
        atom = ParseExpression(tokeniser, 0);
        
        printf("found expression in parentheses! Printing\n");
        PrettyPrintAST(atom->child, 0);
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_CLOSE) {
            ParserFail(tokeniser, "Missing ')' in expression", TOKEN_SEMICOLON);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    else if(PeekToken(tokeniser->buffer).type == TOKEN_LITERAL_INT) {
        Token number = GetNextTokenAndAdvance(tokeniser);
        
        atom = &nodes[node_count++];
        atom->type = AST_NODE_ATOM;
        atom->int_literal_value = number.value;
    }
    
    return atom;
}

AstNode * ParseExpression(Tokeniser * tokeniser, int min_precedence) {
    AstNode * expression = &nodes[node_count++];
    expression->type = AST_NODE_EXPRESSION;
    
    AstNode * left_hand_side = ParseAtom(tokeniser);
    
    TokenType token_type = PeekToken(tokeniser->buffer).type;
    
    while(IsBinaryOperator(token_type) && ParseletLookUp(token_type).precedence > min_precedence) {
        Token token = GetNextTokenAndAdvance(tokeniser);
        Parselet parselet = ParseletLookUp(token.type);
        OperatorType operator_type = BinaryOperatorType(token_type);
        
        int next_min_precedence = parselet.precedence;
        if(parselet.associativty == ASSOCIATE_LEFT)
            next_min_precedence += 1;
        
        AstNode * right_hand_side = ParseExpression(tokeniser, next_min_precedence);
        
        AstNode * new_left_hand_side = &nodes[node_count++];
        *new_left_hand_side = (AstNode){
            .type          = AST_NODE_BINARY_OPERATOR,
            .operator_type = operator_type,
            .left_child    = left_hand_side,
            .right_child   = right_hand_side,
        };
        left_hand_side = new_left_hand_side;
        
        token_type = PeekToken(tokeniser->buffer).type; 
    }
    
    expression->child = left_hand_side;
    
    return expression;
}

AstNode * ParseStatement(Tokeniser * tokeniser) {
    AstNode * statement = &nodes[node_count++];
    statement->type = AST_NODE_STATEMENT;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_KEYWORD_RETURN) {
        ParserFail(tokeniser, "Expected keyword 'return' at start of statement", 0);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    statement->child = ParseExpression(tokeniser, 1);
    
    // Note(abi): until just ; considered a valid statement, occasionally hitting this error
    //            error will erroneuously throw an 'expected }' error; (i believe)
    if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
        ParserFail(tokeniser, "Expected ';'", 0);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    return statement;
};

AstNode * ParseFunction(Tokeniser * tokeniser) {
    AstNode * function = &nodes[node_count++];
    function->type = AST_NODE_FUNCTION;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_KEYWORD_INT) {
        ParserFail(tokeniser, "Expected keyword int", 0);
        return 0;
    }
    
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_IDENTIFIER) {
        ParserFail(tokeniser, "Expected identifier", 0);
        return 0;
    }
    
    Token identifier = GetNextTokenAndAdvance(tokeniser);
    function->function_name = identifier.string;
    function->function_name_length = identifier.string_length;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_OPEN) {
        ParserFail(tokeniser, "Expected (", 0);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_CLOSE) {
        ParserFail(tokeniser, "Expected )", 0);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_BRACE_OPEN) {
        ParserFail(tokeniser, "Expected {", 0);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    function->child = ParseStatement(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_BRACE_CLOSE) {
        ParserFail(tokeniser, "Expected }", 0);
        return 0;
    }
    
    return function;
}

AstNode * ParseProgram(Tokeniser * tokeniser) {
    AstNode * node = &nodes[node_count++];
    node->type = AST_NODE_PROGRAM;
    //parse function,
    //if the tokens do not form a function, fail
    node->child = ParseFunction(tokeniser);
    
    return node;
}

void PrintDepthTabs(int depth) {
    for(int i = 0; i < depth; ++i) {
        printf("\t");
    }
}

void PrettyPrintAST(AstNode * node, int depth) {
    switch (node->type) {
        case AST_NODE_PROGRAM: {
            printf("PROGRAM\n");
            PrettyPrintAST(node->child, depth);
        } break;
        
        case AST_NODE_FUNCTION: {
            PrintDepthTabs(depth);
            printf("FUNC INT %.*s:\n", node->function_name_length, node->function_name);
            PrintDepthTabs(depth);
            printf("\tparams: ()\n");
            PrintDepthTabs(depth);
            printf("\tbody:\n");
            
            PrettyPrintAST(node->child, depth+2);
        } break;
        
        case AST_NODE_STATEMENT: {
            PrintDepthTabs(depth);
            
            printf("RETURN ");
            // Note(abiab): whilst only printing expressions, depth is irrelevant here
            PrettyPrintAST(node->child, depth);
            printf("\n");
        } break;
        
        case AST_NODE_EXPRESSION: {
            PrettyPrintAST(node->child, depth);
        } break;
        
        case AST_NODE_UNARY_OPERATOR: {
            switch (node->operator_type) {
                case OPERATOR_NEGATE: {
                    printf("NEGATE(");
                } break;
                
                case OPERATOR_BITWISE_COMP: {
                    printf("BITCOMP(");
                } break;
                
                case OPERATOR_MINUS_UNARY: {
                    printf("NEGATIVE(");
                } break;
                
            }
            PrettyPrintAST(node->child, depth);
            printf(")");
        } break;
        
        case AST_NODE_BINARY_OPERATOR: {
            switch (node->operator_type) {
                case OPERATOR_PLUS: {
                    printf("ADD(");
                } break;
                
                case OPERATOR_MULTIPLY: {
                    printf("MULT(");
                } break;
                
                case OPERATOR_MINUS_BINARY: {
                    printf("SUB(");
                } break;
                
                case OPERATOR_DIVIDE: {
                    printf("DIV(");
                } break;
                
                case OPERATOR_MODULO: {
                    printf("MOD(");
                } break;
            }
            PrettyPrintAST(node->left_child, depth);
            printf(", ");
            PrettyPrintAST(node->right_child, depth);
            printf(")");
        } break;
        
        case AST_NODE_ATOM: {
            // factor either "(" <expression> ")" or <unary_operator> <factor>
            if(node->child) {
                PrettyPrintAST(node->child, depth);
            }
            else {
                printf("INT<%d>", node->int_literal_value);
            }
        } break;
    }
}
