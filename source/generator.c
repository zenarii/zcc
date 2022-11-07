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
            // Note(abi): for a return statement MUST CHANGE THIS TOO
            GenerateAsmFromAst(file, node->child);
            fprintf(file, "ret\n");
        } break;
        
        case AST_NODE_EXPRESSION: {
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_UNARY_OPERATOR: {
            // Put inside into %rax
            GenerateAsmFromAst(file, node->child);
            
            switch (node->operator_type) {
                case OPERATOR_NEGATE: {
                    fprintf(file, "cmpq $0, %%rax\n");
                    fprintf(file, "movq $0, %%rax\n");
                    fprintf(file, "setz %%al\n");
                } break;
                
                case OPERATOR_MINUS_UNARY: {
                    fprintf(file, "neg %%rax\n");
                } break;
                
                case OPERATOR_BITWISE_COMP: {
                    fprintf(file, "not %%rax\n");
                } break;
                
                default: {
                    printf("[Error] invalid unary operator in code generation");
                    *(int *)0 = 0;
                }
            }
        } break;
        
        case AST_NODE_BINARY_OPERATOR: {
            GenerateAsmFromAst(file, node->left_child);
            fprintf(file, "push %%rax\n");
            
            GenerateAsmFromAst(file, node->right_child);
            fprintf(file, "pop %%rcx\n");
            
            // Note(abi): left child now stored in %rcx, right child stored in %rax
            
            switch (node->operator_type) {
                case OPERATOR_PLUS: {
                    fprintf(file, "addq %%rcx, %%rax\n"); 
                } break;
                
                case OPERATOR_MULTIPLY: {
                    fprintf(file, "imul %%rcx, %%rax\n");
                } break;
                
                case OPERATOR_MINUS_BINARY: {
                    // Note(abi): subq src, dst runs dst - src and stores in dst
                    fprintf(file, "subq %%rax, %%rcx\n");
                    fprintf(file, "movq %%rcx, %%rax\n");
                } break;
                
                case OPERATOR_DIVIDE: {
                    // Note(abi): idiv dst divides %rdx:%rax by dst
                    // move left into rax, right into rcx
                    fprintf(file, "push %%rax\n");
                    fprintf(file, "movq %%rcx, %%rax\n");
                    fprintf(file, "pop %%rcx\n");
                    
                    fprintf(file, "cqo\n");
                    fprintf(file, "idiv %%rcx\n");
                } break;
            }
        } break;
        
        case AST_NODE_ATOM: {
            if(node->child) {
                // Note(abi): child is either an expression or a unary operator
                GenerateAsmFromAst(file, node->child);
            } else {
                fprintf(file, "movq $%d, %%rax\n", node->int_literal_value); 
            }
        } break;
    }
}