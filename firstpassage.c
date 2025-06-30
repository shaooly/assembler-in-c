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


int name_valid(char *name, int IC, int *error) {
    const char *known_instructions[18] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend"};

    if (strlen(name) > 30) {
        fprintf(stderr, "Error in line %d. The label name is longer than 30 chars.\n", IC);
        *error = 1;

        return 0;
    }
    if (isalpha(name[0]) == 0) {
        fprintf(stderr, "Error in line %d. The first char in label name is a number.\n", IC);
        *error = 1;
        return 0;

    }
    int i =0;
    for (i = 0; name[i] != '\0' && name[i] != '\n'; i++) {
        if (isalnum(name[i]) == 0 && name[i] != '_') {
            if (name[i] == ':' && name[i+1] != '\0') { // check if dots are in the name not in the last positon of name
                fprintf(stderr, "Error in line %d. The char in the %dth position in macro name is not alphanumeric.\n"
                    , IC, i);
                *error = 1;
                return 0;
            }
        }
    }
    // amud 28 lahzor
    if (name[strlen(name) - 1] == '\n') {
        name[strlen(name) - 1] = '\0'; // remove newline if there is any
    }
    for (i = 0; i < 18; i++) {
        if (strcmp(name, known_instructions[i]) == 0) { // if macro name is known instruction name.
            fprintf(stderr, "Error in line %d. The macro name is a known instruction name (%s).", IC,
                known_instructions[i]);
            *error = 1;
            return 0;
        }
    }
    return 1;
}

// remember to add more checks !!!
int is_symbol(char first_word[LINE_LENGTH], int *weird_flag, int IC, int *error) {

    char *possible_instructions[5] = {".data", ".string", ".mat", ".entry", ".extern"};

    if (first_word[strlen(first_word) - 1] == ':') {
        if (name_valid(first_word, IC, error)) {
            return 1;
        }
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

int is_instruction(char first_word[LINE_LENGTH], int IC, int *error) {
    const char *known_instructions[18] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend"};

    int i = 0;
    for (i = 0; i < 18; i++) {
        if (strcmp(first_word, known_instructions[i]) == 0) {
            return 1;
        }
    }
    fprintf(stderr, "Error: unknown instruction '%s' in line %d. \n", first_word, IC);
    *error = 1;
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
    // check for weird case
    for (i = 0; i < 3; i++) {
        if (strcmp(first_word, data_storers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


void insert_to_label(label_list *list, char label_name[LINE_LENGTH], char label_type[LINE_LENGTH], int *error, int DC) {
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
        strcpy(to_add->label_type, label_type);
        to_add->value = DC;
        to_add->next_label = NULL;
        tmp->next_label = to_add;
        return;
    }
    *error = 1;
}

void identify_data(char *data_type, char *data, int *DC, int *error, int IC) {
    char full_data[LINE_LENGTH] = "";

    if (strcmp(data_type, ".string") != 0) {
        while (data != NULL) {
            strcat(full_data, data);
            data = strtok(NULL, " \t\n");
        }
    }
    else {
        while (data != NULL) {
            strcat(full_data, data);
            data = strtok(NULL, "");
        }
        // printf(full_data);
    }
    int i = 0;
    int word_count = 0;
    if (strcmp(data_type, ".data") == 0) { // if contaings numbers and such
        word_count++; // counting the commas, so there will always be n - 1 commas
        while (i < strlen(full_data)) {
            if ((full_data[i] == ',' && full_data[i + 1] == ',') || // shnei psikim beretzef yaani
                (full_data[i] == ',' && i == 0) || // psik the first
                (full_data[i] == ',' && i + 1 == strlen(full_data)) // psik the last :O
                ) {
                *error = 1;
                fprintf(stderr, "You have a problem with your commas :( it's on line %d. Fix it.\n", IC);
                return;
            }
            if (full_data[i] == ',') { // new number and we know not two psikim next to each other
                word_count++; // new mila
            }
            // make sure the plus sign only appears next to a number or at the start of the expression :)
            else if ((full_data[i] == '+' || full_data[i] == '-') && full_data[i - 1] != ',' && i != 0) {
                *error = 1;
                fprintf(stderr, "You have a sign at a bad place :( at line %d\n", IC);
            }
            // check if all are numbers
            // + and - signs give false positive so account for these tooooooooooooooo
            else if ((full_data[i] < '0' || full_data[i] > '9') && full_data[i] != '+' && full_data[i] != '-') {
                *error = 1;
                fprintf(stderr, "You have something that isn't a number in line %d"
                    , full_data[i], IC);
            }
        i++;
        }
    }
    if (strcmp(data_type, ".string") == 0) { // finished not ruining
        // count letters and account for quotes
        i = 0;
        // the condition after the || is for a very very rare case in which
        // the last line of the code is a string data storing decleration
        // without a name
        // for example .string "hello"
        while (full_data[i] != '\n' && full_data[i] != '\0') {
            i++;
        }
        // now i is the length of the string so i-1 is supposed to be "
        if (full_data[0] == '"' && full_data[i-1] == '"') { // check if valid corners
            word_count += i - 1; // minus two quotes + '\0'
            // printf("I would've added %d words\n", word_count);
        }
    }
    if (strcmp(data_type, ".mat") == 0) {
        int first_number;
        int second_number;
        if (sscanf(full_data, "[%d][%d]", &first_number, &second_number) == 2) {
            word_count += first_number * second_number;
        }
        // printf("I would've added %d words\n", word_count);
    }

    //update DC
    *DC += word_count;


}

int main() {
    int DC = 0;
    int IC = 1;
    int exists_error = 0;
    int exists_label = 0;
    FILE *source_asm = fopen("postpre.asm", "r");
    label_list* the_label_list = malloc(sizeof(label_list));
    // define empty head
    the_label_list->next_label = NULL;
    strcpy(the_label_list->label_name,"");

    char line[LINE_LENGTH];

    while (fgets(line, LINE_LENGTH, source_asm) && !exists_error) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        char *first_word = strtok(line_for_tokenisation, " \t\n");
        char *second_word;
        // if we get string as data we want it not to erase the spaces
        // account for this early on

        if (strcmp(first_word, ".string") == 0) { // this is only for the weird symbol case.
            second_word = strtok(NULL, "");
        }
        else {
            second_word = strtok(NULL, " \t\n");
        }
        char *third_word;
        if (second_word) {
            if (strcmp(second_word, ".string") == 0) {
                third_word = strtok(NULL, "");
            }
            else {
                third_word = strtok(NULL, " \t\n");
            }
        }
        // this is for the case of wasting space in assembly and defining a line like this:
        // .data "somedata"
        // for some reason it's possible, in the project handbook i didn't see any refrence to it
        // but i guess i should take care of it too no?
        int weird_symbol_flag = 0; // in case there is a "weird" symbol (data wasting space for example .data)

        exists_label = is_symbol(first_word, &weird_symbol_flag, IC, &exists_error);
        if (is_data_storing(first_word, second_word)) { // 5
            if (exists_label) {
                // in the case that the decleration is only .data, we will call without the name (first_word)
                insert_to_label(the_label_list, first_word, "data", &exists_error, DC); // 6
                if (weird_symbol_flag) {
                    identify_data(first_word, second_word, &DC, &exists_error, IC);
                }
                else {
                    identify_data(second_word, third_word, &DC, &exists_error, IC); // 7
                }
            }
            printf("Identified line %s and updated DC to be %d\n", first_word, DC);
        }
        // non-data (entry and extern) are always weird flag cases
        else if (weird_symbol_flag) { // if not data (line 8 // actually it is always here no need
                if (strcmp(first_word, ".entry") == 0) { // 9
                    IC++;
                    continue;
                }
                // if i was arrogant i would've left this if out of the code because if it reached here
                // and it is a symbol and clearly is not any of the other 4 options so it's for sure
                // insert_to_label(label_list *list, char label_name[LINE_LENGTH], int *error, int DC)
                if (strcmp(first_word, ".extern") == 0) {
                    // 10
                    insert_to_label(the_label_list, second_word, "external", &exists_error, 0);
                }
        }
        else if (exists_label) { // 11
            insert_to_label(the_label_list, first_word, "code", &exists_error, IC);
            }
        else { // instruction without label ! // this means that first word has to be a known instruction
            if (is_instruction(first_word, IC, &exists_error)) { // 12

            }

        }

    IC++;
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
