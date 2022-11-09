// NOTE(Abi): Important todo; some kind of memory management or vector type for ast node lists

typedef enum Associativity Associativity;
enum Associativity {
    ASSOCIATE_INVALID,
    
    ASSOCIATE_RIGHT,
    ASSOCIATE_LEFT,
};

typedef struct Parselet Parselet;
struct Parselet {
    TokenType token;
    int precedence;
    Associativity associativty;
};

static Parselet parselets[] = {
    {TOKEN_PLUS,             11, ASSOCIATE_LEFT},
    {TOKEN_MINUS,            11, ASSOCIATE_LEFT},
    {TOKEN_STAR,             13, ASSOCIATE_LEFT},
    {TOKEN_SLASH_FORWARD,    13, ASSOCIATE_LEFT},
    {TOKEN_OR,                3, ASSOCIATE_LEFT},
    {TOKEN_AND,               5, ASSOCIATE_LEFT},
    {TOKEN_EQUALS,            7, ASSOCIATE_LEFT},
    {TOKEN_NOT_EQUAL,         7, ASSOCIATE_LEFT},
    {TOKEN_LESS_THAN,         9, ASSOCIATE_LEFT},
    {TOKEN_LESS_OR_EQUAL,     9, ASSOCIATE_LEFT},
    {TOKEN_GREATER_THAN,      9, ASSOCIATE_LEFT},
    {TOKEN_GREATER_OR_EQUAL,  9, ASSOCIATE_LEFT},
    {TOKEN_ASSIGN,            1, ASSOCIATE_RIGHT},
};

Parselet ParseletLookUp(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:             return parselets[0];
        case TOKEN_MINUS:            return parselets[1];
        case TOKEN_STAR:             return parselets[2];
        case TOKEN_SLASH_FORWARD:    return parselets[3];
        case TOKEN_OR:               return parselets[4];
        case TOKEN_AND:              return parselets[5];
        case TOKEN_EQUALS:           return parselets[6];
        case TOKEN_NOT_EQUAL:        return parselets[7];
        case TOKEN_LESS_THAN:        return parselets[8];
        case TOKEN_LESS_OR_EQUAL:    return parselets[9];
        case TOKEN_GREATER_THAN:     return parselets[10];
        case TOKEN_GREATER_OR_EQUAL: return parselets[11];
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
    AST_NODE_VARIABLE,
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
    
    OPERATOR_EQUALS,
    OPERATOR_NOT_EQUAL,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_LESS_THAN,
    OPERATOR_LESS_OR_EQUAL,
    OPERATOR_GREATER_THAN,
    OPERATOR_GREATER_OR_EQUAL,
    
    OPERATOR_ASSIGN,
};

typedef enum StatementType StatementType;
enum StatementType {
    STATEMENT_INVALID,
    
    STATEMENT_RETURN,
    STATEMENT_EXPRESSION,
    STATEMENT_DECLARATION,
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
    char * identifier;
    int identifier_length;
    
    // for integer literal
    int int_literal_value;
    
    // for unary operator
    OperatorType operator_type;
    
    StatementType statement_type;
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
            (type == TOKEN_PLUS) ||
            (type == TOKEN_AND) ||
            (type == TOKEN_OR) ||
            (type == TOKEN_EQUALS) ||
            (type == TOKEN_NOT_EQUAL) ||
            (type == TOKEN_GREATER_THAN) ||
            (type == TOKEN_GREATER_OR_EQUAL) ||
            (type == TOKEN_LESS_THAN) ||
            (type == TOKEN_LESS_OR_EQUAL) ||
            (type == TOKEN_ASSIGN));
}

OperatorType UnaryOperatorType(TokenType token_type) {
    switch (token_type) {
        case TOKEN_MINUS:  return OPERATOR_MINUS_UNARY;
        case TOKEN_EXCLAM: return OPERATOR_NEGATE;
        case TOKEN_TILDE:  return OPERATOR_BITWISE_COMP;
    }
}

OperatorType BinaryOperatorType(TokenType token_type) {
    switch (token_type) {
        case TOKEN_STAR:             return OPERATOR_MULTIPLY;
        case TOKEN_PLUS:             return OPERATOR_PLUS;
        case TOKEN_MINUS:            return OPERATOR_MINUS_BINARY;
        case TOKEN_SLASH_FORWARD:    return OPERATOR_DIVIDE;
        case TOKEN_EQUALS:           return OPERATOR_EQUALS;
        case TOKEN_NOT_EQUAL:        return OPERATOR_NOT_EQUAL;
        case TOKEN_OR:               return OPERATOR_OR;
        case TOKEN_AND:              return OPERATOR_AND;
        case TOKEN_LESS_THAN:        return OPERATOR_LESS_THAN;
        case TOKEN_LESS_OR_EQUAL:    return OPERATOR_LESS_OR_EQUAL;
        case TOKEN_GREATER_THAN:     return OPERATOR_GREATER_THAN;
        case TOKEN_GREATER_OR_EQUAL: return OPERATOR_GREATER_OR_EQUAL;
        case TOKEN_ASSIGN:           return OPERATOR_ASSIGN;
    }
}

//
// ~Parsing Functions
//

AstNode * ParseExpression(Tokeniser * tokeniser, int min_precedence);

AstNode * ParseAtom(Tokeniser * tokeniser) {
    AstNode * atom;
    
    if(PeekToken(tokeniser->buffer).type == TOKEN_PARENTHESIS_OPEN) {
        GetNextTokenAndAdvance(tokeniser);
        atom = ParseExpression(tokeniser, 0);
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_CLOSE) {
            ParserFail(tokeniser, "Missing ')' in expression", TOKEN_SEMICOLON);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    else if(IsUnaryOperator(PeekToken(tokeniser->buffer).type)) {
        Token unary_operator = GetNextTokenAndAdvance(tokeniser);
        
        atom = &nodes[node_count++];
        atom->type = AST_NODE_UNARY_OPERATOR;
        atom->operator_type = UnaryOperatorType(unary_operator.type);
        atom->child = ParseAtom(tokeniser);
    }
    else if(PeekToken(tokeniser->buffer).type == TOKEN_LITERAL_INT) {
        Token number = GetNextTokenAndAdvance(tokeniser);
        
        atom = &nodes[node_count++];
        atom->type = AST_NODE_ATOM;
        atom->int_literal_value = number.value;
    }
    else if(PeekToken(tokeniser->buffer).type == TOKEN_IDENTIFIER) {
        Token identifier = GetNextTokenAndAdvance(tokeniser);
        
        atom = &nodes[node_count++];
        atom->type = AST_NODE_VARIABLE;
        atom->identifier = identifier.string;
        atom->identifier_length = identifier.string_length;
    }
    else {
        Token next = PeekToken(tokeniser->buffer);
        if(IsBinaryOperator(next.type)) 
            ParserFail(tokeniser, "Expected int before binary operator", TOKEN_SEMICOLON);
        else
            ParserFail(tokeniser, "Expected int to end expression", TOKEN_SEMICOLON);
        return 0;
    }
    
    return atom;
}

AstNode * ParseExpression(Tokeniser * tokeniser, int min_precedence) {
    AstNode * expression = &nodes[node_count++];
    expression->type = AST_NODE_EXPRESSION;
    
    AstNode * left_hand_side = ParseAtom(tokeniser);
    
    TokenType token_type = PeekToken(tokeniser->buffer).type;
    
    if(token_type == TOKEN_ASSIGN && left_hand_side->type != AST_NODE_VARIABLE) {
        ParserFail(tokeniser, "Cannot assign value to non variable left hand side", 0);
    }
    
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
    
    if(PeekToken(tokeniser->buffer).type == TOKEN_KEYWORD_RETURN) {
        GetNextTokenAndAdvance(tokeniser);
        statement->statement_type = STATEMENT_RETURN;
        statement->right_child = ParseExpression(tokeniser, 0);
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
            ParserFail(tokeniser, "Expected ';' at end of return statement", TOKEN_BRACE_CLOSE);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    else if(PeekToken(tokeniser->buffer).type == TOKEN_KEYWORD_INT) {
        // Note(abi): declaration
        GetNextTokenAndAdvance(tokeniser);
        statement->statement_type = STATEMENT_DECLARATION;
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_IDENTIFIER) {
            ParserFail(tokeniser, "Tried to declare variable with no identifier", TOKEN_SEMICOLON);
            return 0;
        }
        
        Token identifier = GetNextTokenAndAdvance(tokeniser);
        statement->identifier = identifier.string;
        statement->identifier_length = identifier.string_length;
        
        if(PeekToken(tokeniser->buffer).type == TOKEN_ASSIGN) {
            GetNextTokenAndAdvance(tokeniser);
            statement->right_child = ParseExpression(tokeniser, 0);
        }
        else if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
            ParserFail(tokeniser, "Declared variable must be followed by '=' or ';'", TOKEN_SEMICOLON);
        }
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
            ParserFail(tokeniser, "Expected ';' at end of declaration", TOKEN_SEMICOLON);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    else {
        // Note(abi): expression
        statement->statement_type = STATEMENT_EXPRESSION;
        statement->right_child = ParseExpression(tokeniser, 0);
        if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
            ParserFail(tokeniser, "Expected ';' at end of expression", TOKEN_BRACE_CLOSE);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    
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
    function->identifier = identifier.string;
    function->identifier_length = identifier.string_length;
    
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
    
    // Note(abi): Assumes at least one statement present in function
    function->child = ParseStatement(tokeniser);
    AstNode * previous_statement = function->child;
    
    Token next_token = PeekToken(tokeniser->buffer);
    while(next_token.type && next_token.type != TOKEN_BRACE_CLOSE && previous_statement) {
        previous_statement->child = ParseStatement(tokeniser);
        previous_statement = previous_statement->child;
        next_token = PeekToken(tokeniser->buffer);
    }
    
    if(next_token.type == TOKEN_INVALID) {
        ParserFail(tokeniser, "Unexpected end of file in function (todo post name)", 0);
    }
    else if(next_token.type == TOKEN_BRACE_CLOSE) {
        GetNextTokenAndAdvance(tokeniser);
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
            printf("FUNC INT %.*s:\n", node->identifier_length, node->identifier);
            PrintDepthTabs(depth);
            printf("\tparams: ()\n");
            PrintDepthTabs(depth);
            printf("\tbody:\n");
            
            PrettyPrintAST(node->child, depth+2);
        } break;
        
        case AST_NODE_STATEMENT: {
            PrintDepthTabs(depth);
            switch (node->statement_type) {
                case STATEMENT_RETURN: {
                    printf("RETURN ");
                    PrettyPrintAST(node->right_child, depth);
                    printf("\n");
                } break;
                
                case STATEMENT_DECLARATION: {
                    printf("DECL INT %.*s", node->identifier_length, node->identifier);
                    if(node->right_child) {
                        printf(" = ");
                        PrettyPrintAST(node->right_child, depth);
                    }
                    printf("\n");
                } break;
                
                case STATEMENT_EXPRESSION: {
                    PrettyPrintAST(node->right_child, depth);
                    printf("\n");
                } break;
            }
            if(node->child) PrettyPrintAST(node->child, depth);
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
                case OPERATOR_PLUS: printf("ADD("); break;
                case OPERATOR_MULTIPLY: printf("MULT("); break;
                case OPERATOR_MINUS_BINARY: printf("SUB("); break;
                case OPERATOR_DIVIDE: printf("DIV("); break;
                case OPERATOR_MODULO: printf("MOD("); break;
                
                case OPERATOR_EQUALS: printf("EQ("); break;
                case OPERATOR_NOT_EQUAL: printf("NEQ("); break;
                case OPERATOR_AND: printf("AND("); break;
                case OPERATOR_OR: printf("OR("); break;
                case OPERATOR_LESS_THAN: printf("LT("); break;
                case OPERATOR_LESS_OR_EQUAL: printf("LEQ("); break;
                case OPERATOR_GREATER_THAN: printf("GT("); break;
                case OPERATOR_GREATER_OR_EQUAL: printf("GEQ("); break;
                
                case OPERATOR_ASSIGN: printf("ASSIGN("); break;
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
                printf("hi");
            }
            else {
                printf("INT<%d>", node->int_literal_value);
            }
        } break;
        
        case AST_NODE_VARIABLE: {
            printf("VAR INT<%.*s>", node->identifier_length, node->identifier);
        } break;
    }
}
