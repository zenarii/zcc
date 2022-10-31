typedef enum AstNodeType AstNodeType;
enum AstNodeType {
    AST_NODE_INVALID,
    
    AST_NODE_PROGRAM,
    AST_NODE_FUNCTION,
    AST_NODE_STATEMENT,
    AST_NODE_EXPRESSION,
};

typedef enum AstNodeExpressionType AstNodeExpressionType;
enum AstNodeExpressionType {
    EXPRESSION_INVALID,
    
    EXPRESSION_INT_LITERAL,
    EXPRESSION_UNARY_OPERATOR,
};

typedef struct AstNode AstNode;
struct AstNode {
    AstNodeType type;
    int subtype;
    
    AstNode * child;
    
    // for function declaration
    char * function_name;
    int function_name_length;
    
    // for integer literal
    int int_literal_value;
    
    // for unary operator
    char unary_operator_character;
};


// TODO(abi): for now use an array of ast nodes, later make the size editable.
static AstNode nodes[128];
static int node_count;

AstNode * ParseExpression(Tokeniser * tokeniser) {
    AstNode * expression = &nodes[node_count++];
    expression->type = AST_NODE_EXPRESSION;
    
    TokenType token_type = PeekToken(tokeniser->buffer).type;
    if(token_type == TOKEN_LITERAL_INT) {
        Token int_literal = GetNextTokenAndAdvance(tokeniser);
        expression->int_literal_value = int_literal.value;
        expression->subtype = EXPRESSION_INT_LITERAL;
    }
    else if(token_type == TOKEN_MINUS || token_type == TOKEN_TILDE || token_type == TOKEN_EXCLAM) {
        Token unary_operator = GetNextTokenAndAdvance(tokeniser);
        expression->unary_operator_character = *unary_operator.string;
        printf("found %c\n", expression->unary_operator_character);
        expression->child = ParseExpression(tokeniser);
        expression->subtype = EXPRESSION_UNARY_OPERATOR;
    }
    else {
        printf("[Error] Expression requires either int literal or unary operator, handed neither\n");
        return 0;
    }
    
    return expression;
}

AstNode * ParseStatement(Tokeniser * tokeniser) {
    AstNode * statement = &nodes[node_count++];
    statement->type = AST_NODE_STATEMENT;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_KEYWORD_RETURN) {
        printf("[Error] Statement expected keyword 'return'\n");
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    statement->child = ParseExpression(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_SEMICOLON) {
        printf("[Error] Statement expected ';'\n");
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    return statement;
};

AstNode * ParseFunction(Tokeniser * tokeniser) {
    AstNode * function = &nodes[node_count++];
    function->type = AST_NODE_FUNCTION;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_KEYWORD_INT) {
        // TODO(abi): Better errors
        printf("[Error] expected int, didn't get it");
        return 0;
    }
    
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_IDENTIFIER) {
        printf("[Error] expected an identifier, didn't get it");
        return 0;
    }
    
    Token identifier = GetNextTokenAndAdvance(tokeniser);
    function->function_name = identifier.string;
    function->function_name_length = identifier.string_length;
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_OPEN) {
        printf("expected (");
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_PARENTHESIS_CLOSE) {
        printf("expected )");
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_BRACE_OPEN) {
        printf("expected {");
        return 0;
    }
    GetNextTokenAndAdvance(tokeniser);
    
    function->child = ParseStatement(tokeniser);
    
    if(PeekToken(tokeniser->buffer).type != TOKEN_BRACE_CLOSE) {
        printf("expected }");
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
    }
}