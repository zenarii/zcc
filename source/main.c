#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a > b) ? (a) : (b))

#include "char.c"
#include "lexer.c"
#include "parser.c"
#include "generator.c"

int main() {
    // Note(abiab): Load the source file into a string. Currently hard coded
    const char * path = "../test_programs/negate.c";
    char * buffer;
    {
        FILE * file = fopen(path, "r");
        if (file == 0) {
            printf("[Error] unable to open %s\n", path);
            return 1;
        }
        
        fseek(file, 0L, SEEK_END);
        long bytes_read = ftell(file);
        
        fseek(file, 0, SEEK_SET);
        
        buffer = malloc(bytes_read);
        
        fread(buffer, sizeof(char), bytes_read, file);
        
        fclose(file);
        buffer[bytes_read] = '\0';
    }
    
    printf("Lexing code\n%s\n", buffer);
    
    Tokeniser tokeniser = {0};
    tokeniser.buffer = buffer;
    
#if 1
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
    
    const char * output_path = "assembly.s";
    FILE * file = fopen(output_path, "w");
    GenerateAsmFromAst(file, root);
    fclose(file);
    
    system("gcc assembly.s -o out");
    
    return 0;
}