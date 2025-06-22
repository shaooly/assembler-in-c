//
// Created by Shauli on 06/06/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "preasm.h"

extern macro_Linked_list* macro_table;


typedef struct label_list{
    int value;
    char label_type[30];
    char label_name[LINE_LENGTH];
    struct label_list* next_label;
} label_list;


// remember to add more checks !!!
int is_symbol(char first_word[LINE_LENGTH], int *weird_flag) {
    char *possible_instructions[5] = {".data", ".string", ".mat", ".entry", ".extern"};
    if (first_word[strlen(first_word) - 1] == ':') {
        return 1;
    }
    int i = 0;
    for (i = 0; i < 5; i++) { // check if there is an instruction .{} without label (rare)
        if (strcmp(first_word, possible_instructions[i]) == 0) {
            *weird_flag = 1;
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
    // check for weird case
    for (i = 0; i < 3; i++) {
        if (strcmp(first_word, data_storers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void insert_to_label(label_list *list, char label_name[LINE_LENGTH], int *error, int DC) {
    // check if label name is in the list yet
    // checking each time for the next one - so i could add it at the end
    label_list *tmp = list;
    while (tmp->next_label != NULL) {
        if (strcmp(label_name, tmp->label_name) == 0) {
            *error = 1;
            fprintf(stderr, "Can't name two labels in the same name! The problem is with: '%s'\n", label_name);
        }
        tmp = tmp->next_label;
    }

    // means the final one does not exist
    // meaning "should" add name, doesn't throw error
    if (strcmp(label_name, tmp->label_name) != 0) {
        // insert into tmp the current label
        label_list *to_add = malloc(sizeof(label_list));
        strcpy(to_add->label_name, label_name);
        strcpy(to_add->label_type, "data");
        to_add->value = DC;
        to_add->next_label = NULL;
        tmp->next_label = to_add;
        return;
    }
    *error = 1;
}

void identify_data(char *data_type, char *data, int *DC) {
    printf("The data is: ");
    while (data != NULL) {
        printf("%s", data);
        data = strtok(NULL, " \t\n");
    }
    printf(" and is from type %s\n", data_type);
}

int main() {
    int DC = 0;
    int IC = 0;
    int exists_error = 0;
    int exists_label = 0;
    FILE *source_asm = fopen("postpre.asm", "r");
    label_list* the_label_list = malloc(sizeof(label_list));
    // define empty head
    the_label_list->next_label = NULL;
    strcpy(the_label_list->label_name,"");

    char line[LINE_LENGTH];


    while (fgets(line, LINE_LENGTH, source_asm)) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        char *first_word = strtok(line_for_tokenisation, " \t");
        char *second_word = strtok(NULL, " \t\n");
        char *third_word = strtok(NULL, " \t\n");

        // this is for the case of wasting space in assembly and defining a line like this:
        // .data "somedata"
        // for some reason it's possible, in the project handbook i didn't see any refrence to it
        // but i guess i should take care of it too no?
        int weird_symbol_flag = 0; // in case there is a "weird" symbol (data wasting space for example .data)

        exists_label = is_symbol(first_word, &weird_symbol_flag);
        if (is_data_storing(first_word, second_word)) {
            if (exists_label == 1) {
                // in the case that the decleration is only .data, we will call without the name (first_word)
                insert_to_label(the_label_list, first_word, &exists_error, DC); // 6
                if (weird_symbol_flag) {
                    identify_data(first_word, second_word, &DC);
                }
                else {
                    identify_data(second_word, third_word, &DC); // 7

                }
            }
        }
        else { // if not data (line 8

        }

    }

    fclose(source_asm);
    int DCF = DC;
    int ICF = IC;

    // free my boy

    while (the_label_list->next_label != NULL) {
        label_list *tmp = the_label_list->next_label;
        free(the_label_list);
        the_label_list = tmp;
    }
    free(the_label_list);

    if (exists_error) {
        fclose(source_asm);
        exit(1);
    }
    return 0;
}
