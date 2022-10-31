#include <stdio.h>
#include <stdlib.h>

#include "char.c"
#include "lexer.c"

int main() {
    // Note(abiab): Load the source file into a string. Currently hard coded
    const char * path = "../test_programs/return_1.c";
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
    
    Token token = {0};
    int print_limit = 0;
    do {
        token = GetNextTokenAndAdvance(&tokeniser);
        PrintToken(token);
        
        if(print_limit++ > 10) break;
    } while (token.type != TOKEN_INVALID);
    return 0;
}