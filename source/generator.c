#include "hashmap.c"

// TODO(abi): have seperate labels for the different used label types? i.e and, or, if etc
int NewLabel() {
    static int label_number;
    return label_number++;
}

int generation_failed;

//
// ~Code Generation
//

void GenerateAsmFromAst(FILE * file, AstNode * node);

void GenerateAsmShortCircuitOperator(FILE * file, AstNode * node) {
    int label = 0;
    // calculate left child result and store it in %rax
    GenerateAsmFromAst(file, node->left_child);
    
    if(node->operator_type == OPERATOR_OR) {
        fprintf(file, "cmpq $0, %%rax\n");
        fprintf(file, "je _or%d_clause2\n", label);
        // Note(abi): didn't jump, so left_child result is true, so move 1 to %rax
        fprintf(file, "movq $1, %%rax\n");
        fprintf(file, "jmp _or%d_end\n", label);
        // Note(abi): in this case, did jump, so need to check the right child
        fprintf(file, "_or%d_clause2:\n", label);
        GenerateAsmFromAst(file, node->right_child);
        fprintf(file, "cmpq $0, %%rax\n");
        fprintf(file, "movq $0, %%rax\n");
        fprintf(file, "setne %%al\n");
        // Note(abi): jump here if short circuited.
        fprintf(file, "_or%d_end:\n", label);
    }
    else if (node->operator_type == OPERATOR_AND) {
        fprintf(file, "cmpq $0, %%rax\n");
        fprintf(file, "jne _and%d_clause2\n", label);
        // left child 0, so jump to the end, no need to set rax as seen rax is 0.
        fprintf(file, "jmp _and%d_end\n", label);
        
        fprintf(file, "_and%d_clause2:\n", label);
        GenerateAsmFromAst(file, node->right_child);
        fprintf(file, "cmpq $0, %%rax\n");
        fprintf(file, "movq $0, %%rax\n");
        fprintf(file, "setne %%al\n");
        // Note(abi): jump here if short circuited.
        fprintf(file, "_and%d_end:\n", label);
    } 
    else {
        printf("[Internal Error] Bad operator type passed to GeneraterAsmShortCircuitOperator\n");
    }
}

void GenerateAsmBooleanOperator(FILE * file, AstNode * node) {
    // Note(abi): Move left child result into %rax and right child result into %rcx.
    GenerateAsmFromAst(file, node->left_child);
    fprintf(file, "push %%rax\n");
    
    GenerateAsmFromAst(file, node->right_child);
    fprintf(file, "pop %%rcx\n");
    
    switch (node->operator_type) {
        case OPERATOR_EQUALS: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "sete %%al\n");
        } break;
        
        case OPERATOR_NOT_EQUAL: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setne %%al\n");
        } break;
        
        case OPERATOR_LESS_THAN: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setl %%al\n");
        } break;
        
        case OPERATOR_LESS_OR_EQUAL: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setle %%al\n");
        } break;
        
        case OPERATOR_GREATER_THAN: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setg %%al\n");
        } break;
        
        case OPERATOR_GREATER_OR_EQUAL: {
            fprintf(file, "cmpq %%rax, %%rcx\n");
            fprintf(file, "movq $0, %%rax\n");
            fprintf(file, "setge %%al\n");
        } break;
    }
}

void GenerateAsmArithmeticOperator(FILE * file, AstNode * node) {
    // Note(abi): Move left child result into %rax and right child result into %rcx.
    GenerateAsmFromAst(file, node->left_child);
    fprintf(file, "push %%rax\n");
    
    GenerateAsmFromAst(file, node->right_child);
    fprintf(file, "pop %%rcx\n");
    
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
        
        case OPERATOR_MODULO: {
            fprintf(file, "push %%rax\n");
            fprintf(file, "movq %%rcx, %%rax\n");
            fprintf(file, "pop %%rcx\n");
            
            fprintf(file, "cqo\n");
            fprintf(file, "idiv %%rcx\n");
            fprintf(file, "movq %%rdx, %%rax\n");
        } break;
    }
}

static Hashmap map;
static int stack_index;

void GenerateAsmFromAst(FILE * file, AstNode * node) {
    switch (node->type) {
        case AST_NODE_PROGRAM: {
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_FUNCTION: {
            fprintf(file, ".globl %.*s\n", node->identifier_length, node->identifier);
            fprintf(file, "%.*s:\n", node->identifier_length, node->identifier);
            // Note(abi): function prologue
            fprintf(file, "push %%rbp\n");
            fprintf(file, "movq %%rsp, %%rbp\n");
            // Note(abi): Generate Function Body
            GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_DECLARATION: {
            if(HashmapContains(&map, node->identifier, node->identifier_length)) {
                // TODO(abi): generation failed function?
                printf("[Error] %.*s already declared in this scope\n", node->identifier_length, node->identifier);
                generation_failed = 1;
                break;
            }
            
            if(node->right_child) {
                GenerateAsmFromAst(file, node->right_child);
                fprintf(file, "push %%rax\n");
                HashmapPut(&map, node->identifier, node->identifier_length, stack_index);
                stack_index -= 8;
            }
            else {
                fprintf(file, "push $0\n");
                HashmapPut(&map, node->identifier, node->identifier_length, stack_index);
                stack_index -= 8;
            }
            
            if(node->child) GenerateAsmFromAst(file, node->child);
        } break;
        
        case AST_NODE_STATEMENT: {
            switch (node->statement_type) {
                case STATEMENT_RETURN: {
                    GenerateAsmFromAst(file, node->right_child);
                    // Generate function epilogue
                    fprintf(file, "movq %%rbp, %%rsp\n");
                    fprintf(file, "pop %%rbp\n");
                    
                    fprintf(file, "ret\n");
                } break;
                
                case STATEMENT_EXPRESSION: {
                    GenerateAsmFromAst(file, node->right_child);
                } break;
                
                case STATEMENT_IF: {
                    int label = NewLabel();
                    // Move conditional value into %rax
                    GenerateAsmFromAst(file, node->condition);
                    fprintf(file, "cmpq $0, %%rax\n");
                    
                    if(!node->else_block) {
                        fprintf(file, "je _if%d_end\n", label); //i.e if false jump
                        GenerateAsmFromAst(file, node->if_block);
                        fprintf(file, "_if%d_end:\n", label);
                    }
                    else { 
                        fprintf(file, "je _if%d_else\n", label);
                        GenerateAsmFromAst(file, node->if_block);
                        fprintf(file, "jmp _if%d_end\n", label);
                        
                        fprintf(file, "_if%d_else:\n", label);
                        GenerateAsmFromAst(file, node->else_block);
                        fprintf(file, "_if%d_end:\n", label);
                    }
                } break;
            }
            
            if(node->child) GenerateAsmFromAst(file, node->child);
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
            switch (node->operator_type) {
                case OPERATOR_PLUS:
                case OPERATOR_MINUS_BINARY:
                case OPERATOR_DIVIDE:
                case OPERATOR_MULTIPLY:
                case OPERATOR_MODULO: {
                    GenerateAsmArithmeticOperator(file, node);
                } break;
                
                case OPERATOR_EQUALS:
                case OPERATOR_NOT_EQUAL:
                case OPERATOR_LESS_THAN:
                case OPERATOR_LESS_OR_EQUAL:
                case OPERATOR_GREATER_THAN:
                case OPERATOR_GREATER_OR_EQUAL: {
                    GenerateAsmBooleanOperator(file, node);
                } break;
                
                case OPERATOR_OR:
                case OPERATOR_AND: {
                    GenerateAsmShortCircuitOperator(file, node);
                } break;
                
                case OPERATOR_ASSIGN: {
                    GenerateAsmFromAst(file, node->right_child);
                    AstNode * variable = node->left_child;
                    if(!HashmapContains(&map, variable->identifier, variable->identifier_length)) {
                        printf("[Error] %.*s undeclared\n", variable->identifier_length, variable->identifier);
                        generation_failed = 1;
                    }
                    int stack_index = HashmapGet(&map, variable->identifier, variable->identifier_length);
                    fprintf(file, "movq %%rax, %d(%%rbp)\n", stack_index);
                } break;
                
                case OPERATOR_TERNARY: {
                    int label = NewLabel();
                    // move condition into %%rax
                    GenerateAsmFromAst(file, node->left_child);
                    fprintf(file, "cmpq $0, %%rax\n");
                    fprintf(file, "je _ternary%d_false\n", label);
                    GenerateAsmFromAst(file, node->left_child->if_block);
                    fprintf(file, "jmp _ternary%d_end\n", label);
                    fprintf(file, "_ternary%d_false:\n", label);
                    GenerateAsmFromAst(file, node->right_child);
                    fprintf(file, "_ternary%d_end:\n", label);
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
        
        case AST_NODE_VARIABLE: {
            int stack_index = HashmapGet(&map, node->identifier, node->identifier_length);
            fprintf(file, "movq %d(%%rbp), %%rax\n", stack_index);
        } break;
    }
}