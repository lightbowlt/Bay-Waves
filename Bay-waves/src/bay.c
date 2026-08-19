#include <stdio.h>
#include "include/bay_token.h"
#include "include/bay_lexer.h"


int main(int argc, char *argv[]) {
    lexer_T* lexer = init_lexer(
        "var_is name equals \"Bay\".\n"
        "say(name).\n");

    token_T* token = (void*)0;
    
    while ((token = lexer_get_next_token(lexer) != (void*)0)) {
            printf("Token(%d,%s)\n", token->type, token->value);
    }

    return 0;

}