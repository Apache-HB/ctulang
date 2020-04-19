#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

char* strdup(const char* str)
{
    int n = strlen(str) + 1;
    char* out = malloc(n);
    if(out)
        memcpy(out, str, n);
    
    return out;
}

#include "lex.h"
#include "parse.h"

void PrintStrVec(vec_str_t* vec, const char* join)
{
    int i = vec_str_size(*vec);
    int j = 0;
    while(j < i)
    {
        if(j != 0)
            printf("%s", join);

        printf("%s", vec_str_get(*vec, j++));
    }
}

void PrintNode(Node* node)
{
    if(!node)
        return;

    if(node->type == NodeTypeImportDecl)
    {
        printf("#include \"");

        PrintStrVec(&node->importDecl.path, "/");

        printf(".h\"\n");

        if(node->importDecl.alias)
        {
            printf("namespace %s = ", node->importDecl.alias);
            PrintStrVec(&node->importDecl.path, "::");
            printf(";\n");
        }
    }
}

int main(int argc, const char** argv)
{
    Lexer lex;

    if(argc > 1)
    {
        lex = NewLexer(fopen(argv[1], "r+"));
    }
    else
    {
        lex = NewLexer(stdin);
    }
    Parser parse = NewParser(&lex);

    for(;;)
    {
        Node* n = ParserNext(&parse);
        PrintNode(n);
    }

    return 0;
}