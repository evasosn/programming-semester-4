#include "forth.h"
#include "words.h"

#include <stdio.h>
#include <string.h>

#define MAX_DATA 16384
#define MAX_STACK 16384
#define MAX_RETURN 16384

int main(int argc, char** argv)
{
    FILE* input;
    struct forth forth = {0};
    forth_init(&forth, stdin, MAX_DATA, MAX_STACK, MAX_RETURN);
    words_add(&forth);
    int end = 0;
    for (int i = 1; i < argc; i++){
        input = fopen(argv[i],"r");
        if (input) {
            forth.input = input;
            forth_run(&forth);
            end = i;
        }
    }
    //printf("%d\n", end);
    if (argc == 1  || (strcmp(argv[argc - 1], "-") == 0 && (end != argc - 1))) {
        forth.input = stdin;
        forth_run(&forth);
    }
    forth_free(&forth);
    return 0;
}
