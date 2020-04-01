#include "forth.h"
#include "words.h"

#include <stdio.h>
#include <string.h>

#define MAX_DATA 16384
#define MAX_STACK 16384
#define MAX_RETURN 16384

int main(int argc, char** argv) {
    FILE* input;
    struct forth forth = {0};
    forth_init(&forth, stdin, MAX_DATA, MAX_STACK, MAX_RETURN);
    words_add(&forth);
    for (int i = 1; i < argc; i++) {
        input = fopen(argv[i],"r");
        if (input) {
            forth.input = input;
        }
        else if (strcmp(argv[i], "-") == 0) {
            forth.input = stdin;
        }
        else {
            printf("Error: there is no file %s\n", argv[i]);
            continue;
        }
        forth_run(&forth);
    }
    if (argc == 1) {
        forth.input = stdin;
        forth_run(&forth);
    }
    forth_free(&forth);
    return 0;
}
