//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


struct Linked_List{
    char instruction[81];
    struct Linked_List *next_instruction;
};

struct macro_Linked_list {
    char name[30];
    struct Linked_List *first_instruction;
    struct macro_Linked_list *next_macro;
};

int name_valid(char *name, int IC) {
    const char *known_instructions[18] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend"};

    if (strlen(name) > 30) {
        fprintf(stderr, "Error in line %d. The macro name is longer than 30 chars.\n", IC);
        return 0;
    }
    if (isalpha(name[0]) == 0) {
        fprintf(stderr, "Error in line %d. The first char in macro name is a number.\n", IC);
        return 0;
    }
    for (int i = 0; i == '\0'; i++) {
        if (isalnum(name[i]) == 0) {
            fprintf(stderr, "Error in line %d. The char in the %dth position in macro name is not alphanumeric.\n"
                , IC, i);
            return 0;
        }
    }
    // amud 28 lahzor
    if (name[strlen(name) - 1] == '\n') {
        name[strlen(name) - 1] = '\0'; // remove newline if there is any
    }
    for (int i = 0; i < 18; i++) {
        if (strcmp(name, known_instructions[i]) == 0) { // if macro name is known instruction name.
            fprintf(stderr, "Error in line %d. The macro name is a known instruction name (%s).", IC,
                known_instructions[i]);
            return 0;
        }
    }
    return 1;
}

int main(void) {
    char *filename = "source.asm";
    int IC = 1;
    FILE *source_asm = fopen(filename, "r");
    FILE *post_pre_asm = fopen("postpre.asm", "w");
    char line[81]; // assuming max line length and is 81 characters long including "\n"

    while (fgets(line, 81, source_asm)) {
        // check if line is longer than 81 chars
        if (strstr(line, "\n") == NULL) { // reached end of fgets count amount of chars without end line.
            if (fgetc(source_asm) == EOF) { // when last line doesn't have "\n"
                exit(9);
            }
            fprintf(stderr, "Error in line %d. The instruction is longer than 80 chars.\n", IC);
            exit(1);
        }

        int i = 0;
        for (i = 0; line[i] == ' ' || (int)line[i] == '\t'; i++) {} // advance i to the first non tab or space char
        char *first_word = strtok(&line[i], " ");
        char *second_word = strtok(NULL, " \t");
        char *third_word = strtok(NULL, " \t");
        // printf("the first word is %s and the second word is %s and third word is %s \n", first_word, second_word, third_word);
        // this isn't such a good way to parse the instructions
        // in the actual not in the pre i will do it differently
        struct macro_Linked_list mcrl;
        int exists_mcro = 0;
        // check if first word of the instruction is mcro
        // check if "third" word is null (meaning the instruction is mcro {name}\n
        // handle space after name (is ok)
        if (strcmp(first_word, "mcro") == 0 && (third_word == NULL || third_word[0] == '\n')) { //
            if (name_valid(second_word, IC) == 0) {
                exit(1);
            }
            printf("all test passed. Will create macro with name %s", second_word);
            //exists_mcro = 1;
            //mcrl.first_instruction = NULL;
            //mcrl.next_macro = NULL;
            //strcpy(mcrl.name, second_word);
        }
        IC++;
    }
    fclose(source_asm);
    fclose(post_pre_asm);
    exit(9);
}