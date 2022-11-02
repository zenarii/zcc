#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a > b) ? (a) : (b))

#include "char.c"
#include "lexer.c"
#include "parser.c"
#include "generator.c"

void Compile(const char * path, const char * output_path, int delete_asm) {
    
    char * buffer;
    {
        FILE * file = fopen(path, "r");
        if (file == 0) {
            printf("[Error] unable to open %s\n", path);
            return;
        }
        
        fseek(file, 0L, SEEK_END);
        long bytes_read = ftell(file);
        
        fseek(file, 0, SEEK_SET);
        
        buffer = malloc(bytes_read);
        
        fread(buffer, sizeof(char), bytes_read, file);
        
        fclose(file);
        buffer[bytes_read] = '\0';
    }
    
    //printf("Compiling code\n%s\n", buffer);
    
    Tokeniser tokeniser = {0};
    tokeniser.buffer = buffer;
    
#if 0
    Token token = {0};
    
    do {
        token = GetNextTokenAndAdvance(&tokeniser);
        PrintToken(token);
    } while (token.type != TOKEN_INVALID);
    
    tokeniser.buffer = buffer;
#endif
    
    AstNode * root = ParseProgram(&tokeniser);
    
#if 1
    PrettyPrintAST(root, 0);
#endif
    
    const char * intermediate_assembly_path = "assembly.s";
    FILE * file = fopen(intermediate_assembly_path, "w");
    GenerateAsmFromAst(file, root);
    fclose(file);
    
    char gcc_call[1024];
    sprintf(gcc_call, "gcc %s -o %s", intermediate_assembly_path, output_path);
    
    system(gcc_call);
    //system("cat assembly.s");
    if(delete_asm) {
        system("rm assembly.s");
    }
}

int main(int argc, char * argv[]) {
    char * output_path = "out";
    int delete_asm = 0;
    
    for(int i = 1; i < argc; i++) {
        if (StringCompareN("-o", argv[i], 2)) {
            output_path = argv[i+1];
            ++i;
        }
        else if (StringCompareN("--delete-asm", argv[i], 12)) {
            delete_asm = 1;
        }
        else {
            Compile(argv[i], output_path, delete_asm);
        }
    }
    
    return 0;
}