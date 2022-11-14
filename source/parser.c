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
    {TOKEN_PLUS,             13, ASSOCIATE_LEFT},
    {TOKEN_MINUS,            13, ASSOCIATE_LEFT},
    {TOKEN_STAR,             15, ASSOCIATE_LEFT},
    {TOKEN_SLASH_FORWARD,    15, ASSOCIATE_LEFT},
    {TOKEN_PERCENT,          15, ASSOCIATE_LEFT},
    {TOKEN_OR,                5, ASSOCIATE_LEFT},
    {TOKEN_AND,               7, ASSOCIATE_LEFT},
    {TOKEN_EQUALS,            9, ASSOCIATE_LEFT},
    {TOKEN_NOT_EQUAL,         9, ASSOCIATE_LEFT},
    {TOKEN_LESS_THAN,        11, ASSOCIATE_LEFT},
    {TOKEN_LESS_OR_EQUAL,    11, ASSOCIATE_LEFT},
    {TOKEN_GREATER_THAN,     11, ASSOCIATE_LEFT},
    {TOKEN_GREATER_OR_EQUAL, 11, ASSOCIATE_LEFT},
    {TOKEN_ASSIGN,            1, ASSOCIATE_RIGHT},
    {TOKEN_QUESTION,          3, ASSOCIATE_RIGHT},
};

Parselet ParseletLookUp(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:             return parselets[0];
        case TOKEN_MINUS:            return parselets[1];
        case TOKEN_STAR:             return parselets[2];
        case TOKEN_SLASH_FORWARD:    return parselets[3];
        case TOKEN_PERCENT:          return parselets[4];
        case TOKEN_OR:               return parselets[5];
        case TOKEN_AND:              return parselets[6];
        case TOKEN_EQUALS:           return parselets[7];
        case TOKEN_NOT_EQUAL:        return parselets[8];
        case TOKEN_LESS_THAN:        return parselets[9];
        case TOKEN_LESS_OR_EQUAL:    return parselets[10];
        case TOKEN_GREATER_THAN:     return parselets[11];
        case TOKEN_GREATER_OR_EQUAL: return parselets[12];
        case TOKEN_ASSIGN:           return parselets[13];
        case TOKEN_QUESTION:         return parselets[14];
        
        default: {
            *(int *)0 = 0;
        }
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
    AST_NODE_DECLARATION,
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
    
    OPERATOR_TERNARY,
};

typedef enum StatementType StatementType;
enum StatementType {
    STATEMENT_INVALID,
    
    STATEMENT_RETURN,
    STATEMENT_EXPRESSION,
    STATEMENT_IF,
    STATEMENT_ELSE,
};

typedef struct AstNode AstNode;
struct AstNode {
    AstNodeType type;
    
    // for statements/declarations/functions, next statement
    // for unary operators, 
    AstNode * child;
    
    // TODO(abi): for operators
    AstNode * left_child;
    AstNode * right_child;
    OperatorType operator_type;
    
    // for functions,  declarations
    char * identifier;
    int identifier_length;
    
    int int_literal_value;
    
    StatementType statement_type;
    // for if statements
    AstNode * condition;
    AstNode * if_block;
    AstNode * else_block;
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
            (type == TOKEN_ASSIGN) ||
            (type == TOKEN_PERCENT) ||
            (type == TOKEN_QUESTION));
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
        case TOKEN_SLASH_FORWARD:    return OPERATOR_DIVIDE;
        case TOKEN_PERCENT:          return OPERATOR_MODULO;
        case TOKEN_PLUS:             return OPERATOR_PLUS;
        case TOKEN_MINUS:            return OPERATOR_MINUS_BINARY;
        case TOKEN_EQUALS:           return OPERATOR_EQUALS;
        case TOKEN_NOT_EQUAL:        return OPERATOR_NOT_EQUAL;
        case TOKEN_OR:               return OPERATOR_OR;
        case TOKEN_AND:              return OPERATOR_AND;
        case TOKEN_LESS_THAN:        return OPERATOR_LESS_THAN;
        case TOKEN_LESS_OR_EQUAL:    return OPERATOR_LESS_OR_EQUAL;
        case TOKEN_GREATER_THAN:     return OPERATOR_GREATER_THAN;
        case TOKEN_GREATER_OR_EQUAL: return OPERATOR_GREATER_OR_EQUAL;
        case TOKEN_ASSIGN:           return OPERATOR_ASSIGN;
        case TOKEN_QUESTION:         return OPERATOR_TERNARY;
    }
}

//
// ~Parsing Functions
//
void PrettyPrintAST(AstNode *, int);
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
        
        
        // Note(abi): ternary-if special case
        //            "? expr :" is functionally a binary operator of precedence ?
        if(operator_type == OPERATOR_TERNARY) {
            left_hand_side->if_block = ParseExpression(tokeniser, 0);
            if(PeekToken(tokeniser->buffer).type != TOKEN_COLON) {
                ParserFail(tokeniser, "Ternary expression must contain ':'", TOKEN_SEMICOLON);
                return 0;
            }
            GetNextTokenAndAdvance(tokeniser);
        }
        
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
    
    TokenType next_token_type = PeekToken(tokeniser->buffer).type;
    if(next_token_type == TOKEN_KEYWORD_RETURN) {
        GetNextTokenAndAdvance(tokeniser);
        statement->statement_type = STATEMENT_RETURN;
        statement->right_child = ParseExpression(tokeniser, 0);
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
            ParserFail(tokeniser, "Expected ';' at end of return statement", TOKEN_BRACE_CLOSE);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
    }
    else if(next_token_type == TOKEN_KEYWORD_IF)  {
        GetNextTokenAndAdvance(tokeniser);
        
        statement->statement_type = STATEMENT_IF;
        if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_OPEN) {
            ParserFail(tokeniser, "Require '(' after 'if' keyword\n", TOKEN_SEMICOLON);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
        
        statement->condition = ParseExpression(tokeniser, 0);
        
        if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_CLOSE) {
            ParserFail(tokeniser, "Require ')' to close if condition\n", TOKEN_SEMICOLON);
            return 0;
        }
        GetNextTokenAndAdvance(tokeniser);
        
        statement->if_block = ParseStatement(tokeniser);
        
        if(PeekToken(tokeniser->buffer).type == TOKEN_KEYWORD_ELSE) {
            GetNextTokenAndAdvance(tokeniser);
            statement->else_block = ParseStatement(tokeniser);
        }
    }
    else if(next_token_type == TOKEN_KEYWORD_ELSE) {
        ParserFail(tokeniser, "Unexpected else statement without matching if\n", TOKEN_SEMICOLON);
        return 0;
    } else {
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

AstNode * ParseDeclaration(Tokeniser * tokeniser) {
    AstNode * declaration = &nodes[node_count++];
    declaration->type = AST_NODE_DECLARATION;
    
    // TODO(abiab): Asserts?
    if(PeekToken(tokeniser->buffer).type != TOKEN_KEYWORD_INT) {
        printf("[Internal Error] Parsing declaration but first token wasn't INT keyword\n");
        parse_failed = 1;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_IDENTIFIER) {
        ParserFail(tokeniser, "Tried to declare variable with no identifier", TOKEN_SEMICOLON);
        return 0;
    }
    
    Token identifier = GetNextTokenAndAdvance(tokeniser);
    declaration->identifier = identifier.string;
    declaration->identifier_length = identifier.string_length;
    
    if(PeekToken(tokeniser->buffer).type == TOKEN_ASSIGN) {
        GetNextTokenAndAdvance(tokeniser);
        declaration->right_child = ParseExpression(tokeniser, 0);
    }
    else if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
        ParserFail(tokeniser, "Declared variable must be followed by '=' or ';'", TOKEN_SEMICOLON);
    }
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
        ParserFail(tokeniser, "Expected ';' at end of declaration", TOKEN_SEMICOLON);
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    return declaration;
}

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
    
    // Note(abi): Assumes at least one statement or declaration present in function
    function->child = PeekToken(tokeniser->buffer).type == TOKEN_KEYWORD_INT ? ParseDeclaration(tokeniser) : ParseStatement(tokeniser);
    AstNode * previous_statement = function->child;
    
    Token next_token = PeekToken(tokeniser->buffer);
    while(next_token.type && next_token.type != TOKEN_BRACE_CLOSE && previous_statement) {
        previous_statement->child = (next_token.type == TOKEN_KEYWORD_INT) ? ParseDeclaration(tokeniser) : ParseStatement(tokeniser);
        
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
        
        case AST_NODE_DECLARATION: {
            PrintDepthTabs(depth);
            
            printf("DECL INT %.*s", node->identifier_length, node->identifier);
            if(node->right_child) {
                printf(" = ");
                PrettyPrintAST(node->right_child, depth);
            }
            printf("\n");
            
            if(node->child) PrettyPrintAST(node->child, depth);
        } break;
        
        case AST_NODE_STATEMENT: {
            PrintDepthTabs(depth);
            switch (node->statement_type) {
                case STATEMENT_RETURN: {
                    printf("RETURN ");
                    PrettyPrintAST(node->right_child, depth);
                    printf("\n");
                } break;
                
                case STATEMENT_IF: {
                    printf("IF ");
                    PrettyPrintAST(node->condition, depth);
                    printf("\n");
                    PrettyPrintAST(node->if_block, depth+1);
                    
                    if(node->else_block) {
                        PrintDepthTabs(depth);
                        printf("ELSE\n");
                        PrettyPrintAST(node->else_block, depth+1);
                    }
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
            if(node->operator_type == OPERATOR_TERNARY) {
                printf("TERNARY(cond: ");
                PrettyPrintAST(node->left_child, depth);
                printf(", e1: ");
                PrettyPrintAST(node->left_child->if_block, depth);
                printf(", e2: ");
                PrettyPrintAST(node->right_child, depth);
                printf(")");
                break;
            }
            
            switch (node->operator_type) {
                case OPERATOR_PLUS: printf("+("); break;
                case OPERATOR_MULTIPLY: printf("*("); break;
                case OPERATOR_MINUS_BINARY: printf("-("); break;
                case OPERATOR_DIVIDE: printf("/("); break;
                case OPERATOR_MODULO: printf("%("); break;
                
                case OPERATOR_EQUALS: printf("==("); break;
                case OPERATOR_NOT_EQUAL: printf("!=("); break;
                case OPERATOR_AND: printf("&&("); break;
                case OPERATOR_OR: printf("||("); break;
                case OPERATOR_LESS_THAN: printf("<("); break;
                case OPERATOR_LESS_OR_EQUAL: printf("<=("); break;
                case OPERATOR_GREATER_THAN: printf(">("); break;
                case OPERATOR_GREATER_OR_EQUAL: printf(">=("); break;
                
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
