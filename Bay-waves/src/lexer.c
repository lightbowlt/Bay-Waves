#include "include/bay_lexer.h"
#include <stdlib.h>
#

lexer_T* int_lexer(char* contents) {
    lexer_T* lexer = calloc(1,sizeof(struct LEXER_STRUCT));
    lexer->contents = contents;
    lexer->i = 0;
    lexer->c = contents[lexer->i];

    return lexer;

}

void 