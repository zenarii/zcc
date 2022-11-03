typedef enum AstNodeType AstNodeType;
enum AstNodeType {
    AST_NODE_INVALID,
    
    AST_NODE_PROGRAM,
    AST_NODE_FUNCTION,
    AST_NODE_STATEMENT,
    AST_NODE_EXPRESSION,
    AST_NODE_BINARY_OPERATOR,
    AST_NODE_UNARY_OPERATOR,
    AST_NODE_FACTOR,
    AST_NODE_TERM,
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

// TODO(abiab): may require different skupping of tokens etc depending on where the error is thrown.
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

//
// ~Parsing Functions
//

AstNode * ParseExpression(Tokeniser * tokeniser);

AstNode * ParseFactor(Tokeniser * tokeniser) {
    AstNode * factor = &nodes[node_count++];
    factor->type = AST_NODE_FACTOR;
    
    if(PeekToken(tokeniser->buffer).type == TOKEN_PARENTHESIS_OPEN) {
        // Note(abi): evaluate the expression within the paranthesis
        GetNextTokenAndAdvance(tokeniser);
        
        factor->child = ParseExpression(tokeniser);
        
        if(GetNextTokenAndAdvance(tokeniser).type != TOKEN_PARENTHESIS_CLOSE) {
            ParserFail(tokeniser, 
                       "Expression in parantheses requires closing ')'", 
                       TOKEN_SEMICOLON);
        }
    }
    else if (IsUnaryOperator(PeekToken(tokeniser->buffer).type)) {
        Token unary_operator_token = GetNextTokenAndAdvance(tokeniser);
        AstNode * unary_operator = &nodes[node_count++];
        
        unary_operator->type = AST_NODE_UNARY_OPERATOR;
        
        switch(unary_operator_token.type) {
            case TOKEN_MINUS: unary_operator->operator_type = OPERATOR_MINUS_UNARY; break;
            case TOKEN_EXCLAM: unary_operator->operator_type = OPERATOR_NEGATE; break;
            case TOKEN_TILDE: unary_operator->operator_type = OPERATOR_BITWISE_COMP; break;
            default: { 
                ParserFail(tokeniser, "Invalid unary operator", TOKEN_SEMICOLON); 
                return 0;
                break;
            }
        }
        
        unary_operator->child = ParseFactor(tokeniser);
        factor->child = unary_operator;
    }
    else if (PeekToken(tokeniser->buffer).type == TOKEN_LITERAL_INT) {
        Token int_literal = GetNextTokenAndAdvance(tokeniser);
        factor->int_literal_value = int_literal.value;
    }
    else {
        ParserFail(tokeniser, "Expected integer literal", TOKEN_SEMICOLON);
        return 0;
    }
    
    return factor;
}

AstNode * ParseTerm(Tokeniser * tokeniser) {
    AstNode * term = &nodes[node_count++];
    term->type = AST_NODE_TERM;
    
    AstNode * factor = ParseFactor(tokeniser);
    
    TokenType token_type = PeekToken(tokeniser->buffer).type;
    // Note(abi): There are more factors
    while(token_type == TOKEN_STAR || token_type == TOKEN_SLASH_FORWARD) {
        OperatorType operator_type = (token_type == TOKEN_STAR) ?
            OPERATOR_MULTIPLY : OPERATOR_DIVIDE;
        
        GetNextTokenAndAdvance(tokeniser);
        
        AstNode * next_factor = ParseFactor(tokeniser);
        AstNode * new_factor = &nodes[node_count++];
        *new_factor = (AstNode) {
            .type          = AST_NODE_BINARY_OPERATOR,
            .operator_type = operator_type,
            .left_child    = factor,
            .right_child   = next_factor,
        };
        factor = new_factor;
        
        token_type = PeekToken(tokeniser->buffer).type;
    }
    
    term->child = factor;
    
    return term;
};

AstNode * ParseExpression(Tokeniser * tokeniser) {
    AstNode * expression = &nodes[node_count++];
    expression->type = AST_NODE_EXPRESSION;
    
    AstNode * term = ParseTerm(tokeniser);
    
    TokenType token_type = PeekToken(tokeniser->buffer).type;
    // Note(abi): there are more terms
    while(token_type == TOKEN_MINUS || token_type == TOKEN_PLUS) {
        OperatorType operator_type = (token_type == TOKEN_MINUS) ? 
            OPERATOR_MINUS_BINARY : OPERATOR_PLUS;
        
        GetNextTokenAndAdvance(tokeniser);
        
        AstNode * next_term = ParseTerm(tokeniser);
        AstNode * new_term = &nodes[node_count++];
        *new_term = (AstNode){
            .type          = AST_NODE_BINARY_OPERATOR,
            .operator_type = operator_type,
            .left_child    = term,
            .right_child   = next_term,
        };
        term = new_term;
        
        token_type = PeekToken(tokeniser->buffer).type; 
    }
    
    expression->child = term;
    
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
    
    statement->child = ParseExpression(tokeniser);
    
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
        
        case AST_NODE_TERM: {
            PrettyPrintAST(node->child, depth);
        } break;
        
        case AST_NODE_FACTOR: {
            // factor either "(" <expression> ")" or <unary_operator> <factor>
            if(node->child) {
                PrettyPrintAST(node->child, depth);
            }
            else {
                printf("INT<%d>", node->int_literal_value);
            }
        } break;
        /*
                        case AST_NODE_EXPRESSION: {
                            if (node->subtype == EXPRESSION_UNARY_OPERATOR) {
                                if     (node->unary_operator_character == '-') printf("NEGATE(");
                                else if(node->unary_operator_character == '~') printf("BITWISECOMP(");
                                else if(node->unary_operator_character == '!') printf("LOGNEGATE(");
                                
                                PrettyPrintAST(node->child, depth);
                                printf(")");
                            }
                            else if(node->subtype == EXPRESSION_INT_LITERAL) {
                                printf("INT<%d>", node->int_literal_value);
                            }
                            else {
                                printf("INVALID EXPRESSION!");
                            }
                        } break;
                */
    }
}
