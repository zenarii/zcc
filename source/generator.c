#define OUTPUT_FILE 

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
            // Note(abi): for a return statement
            fprintf(file, "movl ");
            GenerateAsmFromAst(file, node->child);
            fprintf(file, ", %%eax\n");
            fprintf(file, "ret\n");
        } break;
        
        case AST_NODE_EXPRESSION: {
            // Note(abi): prints an integer literal
            fprintf(file, "$%d", node->int_literal_value);
        } break;
    }
}