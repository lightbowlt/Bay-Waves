#include <stdio.h>
#include "include/bay_token.h"
#include "include/bay_lexer.h"


int main(int argc, char *argv[]) {
    lexer_T* lexer = init_lexer(
        "variable name equals \"Yuhithan\".\n"
        "say(name).\n");

    token_T* token = (void*)0;
    
    while ((token = lexer_get_next_token(lexer)) != (void*)0) {
            printf("Token(%d,%s)\n", token->type, token->value);
    }

    return 0;

}