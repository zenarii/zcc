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
        }
        
        case AST_NODE_BINARY_OPERATOR: {
            // TODO
        } break;
        
        case AST_NODE_TERM: {
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_FACTOR: {
            if(node->child) {
                // Note(abi): child is either an expression or a unary operator
                GenerateAsmFromAst(file, node->child);
            } else {
                fprintf(file, "movq $%d, %%rax\n", node->int_literal_value); 
            }
        } break;
    }
}