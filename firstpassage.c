//
// Created by Shauli on 06/06/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "preasm.h"

extern macro_Linked_list* macro_table;


typedef struct label_List{
    int value;
    char label_type[30];
    struct label_List* next_instruction;
} label_List;

int is_symbol(char first_word[LINE_LENGTH]) {
    char *possible_instructions[5] = {".data", ".string", ".mat", ".entry", ".extern"};
    printf("%s\n", first_word);
    if (first_word[strlen(first_word) - 1] == ':') {
        return 1;
    }
    int i = 0;
    for (i = 0; i < 5; i++) { // check if there is an instruction .{} without label (rare)
        if (strcmp(first_word, possible_instructions[i]) == 0) {
            return 1;
        }
    }
    return 0;

}
int is_data_storing(char first_word[LINE_LENGTH], char second_word[LINE_LENGTH]) {
    char *data_storers[3] = {".data", ".string", ".mat"};
    if (first_word[strlen(first_word) - 1] == ':') {
        int i = 0;
        for (i = 0; i < 3; i++) {
            if (strcmp(second_word, data_storers[i]) == 0) {
                return 1;
            }
        }
    }
    int i = 0;
    for (i = 0; i < 3; i++) {
        if (strcmp(second_word, data_storers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
int main() {
    int DC = 0;
    int IC = 0;
    int exists_error = 0;
    int exists_label = 0;
    FILE *source_asm = fopen("postpre.asm", "r");
    char line[LINE_LENGTH];


    while (fgets(line, LINE_LENGTH, source_asm)) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        char *first_word = strtok(line_for_tokenisation, " \t");
        char *second_word = strtok(NULL, " \t\n");
        char *third_word = strtok(NULL, " \t\n");
        if (is_symbol(first_word)) {
            exists_label = 1;
            if (is_data_storing(first_word, second_word)) {
                printf("hi, i'm data storing");
                printf(macro_table->name);
            }

        }
    }
    if (exists_error) {
        fclose(source_asm);
        exit(1);
    }
    fclose(source_asm);
    int DCF = DC;
    int ICF = IC;

    return 0;
}
