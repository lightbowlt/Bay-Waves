#include <stdio.h>
#include "include/bay_token.h"
#include "include/bay_lexer.h"
#include "include/bay_parser.h"


int main(int argc, char *argv[]) {
    lexer_T* lexer = init_lexer(
        "variable name = \"Yuhithan\".\n"
        "say(name).\n");

    token_T* token = (void*)0;
    
    //while ((token = lexer_get_next_token(lexer)) != (void*)0) {
    //        printf("Token(%d,%s)\n", token->type, token->value);
    //}

    parser_T* parser = init_parser(lexer);
    AST_T* root = parser_parse(parser);

    printf("AST Type: %d\n", root->type);
    printf("AST Compound Size: %zu\n", root->compound_size);

    return 0;

}