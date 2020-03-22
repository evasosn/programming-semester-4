
#include "forth.c"
#include "words.c"
#include "minunit.h"

void do_operation(char* operation, cell expected, struct forth* forth);
void do_roperation(char* operation, cell expected, struct forth* forth);
void do_memoperation(char* operation, struct forth* forth);


void do_operation(char* operation, cell expected, struct forth* forth) {
    FILE* input;
    char test[1000];
    sprintf(test,"1 2 3 %s",operation);

    input = fmemopen(test, strlen(test), "r");
    forth_init(forth, input, 1000, 1000, 1000);
    words_add(forth);
    forth_run(forth);
    fclose(input);
    mu_check(*forth_top(forth) == expected);
    forth_free(forth);
}

void do_roperation(char* operation, cell expected, struct forth* forth) {
    FILE* input;
    char test[80];
    sprintf(test,"1 2 3 4 5 + >r >r %s",operation);

    input = fmemopen(test, strlen(test), "r");
    forth_init(forth, input, 1000, 1000, 1000);
    words_add(forth);
    forth_run(forth);
    fclose(input);
    mu_check(*forth_top(forth) == expected);
    forth_free(forth);
}

void do_memoperation(char* operation, struct forth* forth) {
    FILE* input;
    char test[80];
    sprintf(test,": 1 2 ; 3 here %s 6 immediate", operation);

    input = fmemopen(test, strlen(test), "r");
    forth_init(forth, input, 1000, 1000, 1000);
    words_add(forth);
    forth_run(forth);
    fclose(input);
    forth_free(forth);
}

MU_TEST(forth_tests_words) {
    struct forth forth = {0};
    struct word **a;
    do_operation("drop", 2, &forth);
    do_operation("dup", 3, &forth);
    do_operation("+", 5, &forth);
    do_operation("-", -1, &forth);
    do_operation("*", 6, &forth);
    do_operation("/", 0, &forth);
    do_operation("%", 2, &forth);
    do_operation("swap", 2, &forth);
    do_operation("rot", 1, &forth);
    do_operation("-rot", 2, &forth);
    do_operation("show", 3, &forth);
    do_operation("over", 2, &forth);
    do_operation("true", -1, &forth);
    do_operation("false", 0, &forth);
    do_operation("xor", 2^3, &forth);
    do_operation("or", 2|3, &forth);
    do_operation("and", 2&3, &forth);
    do_operation("not", ~3, &forth);
    do_operation("=", 0, &forth);
    do_operation("<", -1, &forth);
    do_operation("within", 0, &forth);

    do_roperation(">r", 1, &forth);
    do_roperation("r>", 3, &forth);
    do_roperation("rshow", 2, &forth);
    do_roperation("i", 9, &forth);

    do_memoperation("@", &forth);
    do_memoperation("!", &forth);
    do_memoperation(",", &forth);
    do_memoperation("word", &forth);
    do_memoperation("find", &forth);
    do_memoperation(">cfa word find", &forth);
    do_operation(": repeat immediate ' branch , swap here @ - ,	dup here @ swap - swap ! ;\
                  : begin immediate here @ ; : while immediate ' 0branch , here @ 0 , ;\
                  : test-loop begin 1 - dup dup while repeat ;\
                  test-loop", 0, &forth);

    forth_init(&forth, stdin, 1000, 1000, 1000);
    a = forth.executing;
    forth.executing -= 1;
    next(&forth);
    mu_check(forth.executing == a);
    forth_free(&forth);
}

MU_TEST(forth_tests_run) {
    struct forth forth = {0};
    FILE *file;
    char test[80];
    sprintf(test, "1 2 3 : + 3 e exit ; ");

    file = fmemopen(test, strlen(test), "r");
    forth_init(&forth, file, 100, 100, 100);
    words_add(&forth);
    mu_check(forth_run(&forth) == FORTH_EOF);
    cell_print(*forth.memory);
}

MU_TEST(forth_tests_readword) {
    FILE *file;
    size_t length;
    char buffer[MAX_WORD+1] = {0};
    char test[MAX_WORD+1] = {0};
    sprintf(test, " 1 2 exit");

    file = fmemopen(test, strlen(test), "r");
    mu_check(read_word(file, 1, buffer, &length) == FORTH_BUFFER_OVERFLOW);
    fclose(file);
    file = fmemopen(test, strlen(test), "r");
    mu_check(read_word(file, MAX_WORD + 1, buffer, &length) == FORTH_OK);
    fclose(file);
}

MU_TEST(forth_tests_top) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    forth_push(&forth, 123);

    mu_check(*forth_top(&forth) == 123);
}

MU_TEST(forth_tests_nocompileword) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    words_add(&forth);
    const char* ErrorSquare[] = { "dup", "e", "exit", 0 };
    mu_check(forth_add_compileword(&forth, "err_square", ErrorSquare) == 1);
}

MU_TEST(forth_tests_init_free) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);

    mu_check(forth.memory == forth.memory_free);
    mu_check(forth.memory != NULL);
    mu_check(forth.sp0 == forth.sp);
    mu_check(forth.sp0 != NULL);

    forth_free(&forth);
}

MU_TEST(forth_tests_align) {
    mu_check(align(8, 8) == 8);
    mu_check(align(9, 8) == 16);
    mu_check(align(7, 8) == 8);
}

MU_TEST(forth_tests_push_pop) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    forth_push(&forth, 123);

    mu_check(forth.sp > forth.sp0);
    mu_check(forth_pop(&forth) == 123);
    mu_check(forth.sp0 == forth.sp);
}

MU_TEST(forth_tests_emit) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    forth_emit(&forth, 123);

    mu_check(forth.memory_free > forth.memory);
    mu_check(*forth.memory == 123);
}

MU_TEST(forth_tests_codeword) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);

    mu_check(forth.latest == NULL);

    struct word *w1 = word_add(&forth, strlen("TEST1"), "TEST1");
    forth_emit(&forth, 123);
    mu_check(forth.latest == w1);

    struct word *w2 = word_add(&forth, strlen("TEST2"), "TEST2");
    mu_check((*(cell*)word_code(w1)) == 123);
    mu_check((void*)w2 > word_code(w1));
    mu_check(forth.latest == w2);

    mu_check(word_find(forth.latest, strlen("TEST1"), "TEST1") == w1);
    mu_check(word_find(forth.latest, strlen("TEST2"), "TEST2") == w2);
    mu_check(word_find(forth.latest, strlen("TEST"), "TEST") == NULL);
}

MU_TEST(forth_tests_compileword) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    words_add(&forth);

    const struct word *dup = word_find(forth.latest, strlen("dup"), "dup");
    const struct word *mul = word_find(forth.latest, strlen("*"), "*");
    const struct word *exit = word_find(forth.latest, strlen("exit"), "exit");
    const struct word *square = word_find(forth.latest, strlen("square"), "square");
    mu_check(square);
    struct word **words = (struct word**)word_code(square);
    mu_check(words[0] == dup);
    mu_check(words[1] == mul);
    mu_check(words[2] == exit);
    struct word *w1 = word_add(&forth, strlen("TEST1"), "TEST1");
    mu_check((void*)w1 > (void*)(words+2));
}

MU_TEST(forth_tests_literal) {
    struct forth forth = {0};
    forth_init(&forth, stdin, 100, 100, 100);
    words_add(&forth);

    const struct word *literal = word_find(forth.latest, strlen("lit"), "lit");
    const struct word *exit = word_find(forth.latest, strlen("exit"), "exit");
    struct word *test = word_add(&forth, strlen("TEST"), "TEST");
    test->compiled = true;
    forth_emit(&forth, (cell)literal);
    forth_emit(&forth, 4567);
    forth_emit(&forth, (cell)exit);

    forth_run_word(&forth, test);
    cell c = forth_pop(&forth);
    mu_check(c == 4567);
}

MU_TEST_SUITE(forth_tests) {
    MU_RUN_TEST(forth_tests_words);
    MU_RUN_TEST(forth_tests_run);
    MU_RUN_TEST(forth_tests_readword);
    MU_RUN_TEST(forth_tests_top);
    MU_RUN_TEST(forth_tests_nocompileword);
    MU_RUN_TEST(forth_tests_init_free);
    MU_RUN_TEST(forth_tests_align);
    MU_RUN_TEST(forth_tests_push_pop);
    MU_RUN_TEST(forth_tests_emit);
    MU_RUN_TEST(forth_tests_codeword);
    MU_RUN_TEST(forth_tests_compileword);
    MU_RUN_TEST(forth_tests_literal);
}
