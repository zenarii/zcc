#define OUTPUT_FILE 

void GenerateExpression(FILE * file, AstNode * expression) {
    if (expression->subtype == EXPRESSION_INT_LITERAL) {
        fprintf(file, "movq $%d, %%rax\n", expression->int_literal_value);
    }
    else if(expression->subtype == EXPRESSION_UNARY_OPERATOR) {
        GenerateExpression(file, expression->child);
        
        // Note(abi): Negation
        if(expression->unary_operator_character == '-') {
            fprintf(file, "neg %%rax\n");
        }
        else if(expression->unary_operator_character == '~') {
            fprintf(file, "not %%rax\n");
        }
        else if(expression->unary_operator_character == '!') {
            fprintf(file, "cmpq $0, %%rax\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setz %%al\n");
        }
        else {
            printf("[ERROR] Unary operator found but invalid character given. This should not be seen by a user");
        }
    }
}

void GenerateAsmFromAst(FILE * file, AstNode * node) {
    switch (node->type) {
        case AST_NODE_PROGRAM: {
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_FUNCTION: {
            fprintf(file, ".globl %.*s\n", node->function_name_length, node->function_name);
            fprintf(file, "%.*s:\n", node->function_name_length, node->function_name);
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_STATEMENT: {
            // Note(abi): for a return statement MUST CHANGE THIS TOO
            GenerateAsmFromAst(file, node->child);
            fprintf(file, "ret\n");
        } break;
        
        case AST_NODE_EXPRESSION: {
            GenerateExpression(file, node);
        } break;
    }
}