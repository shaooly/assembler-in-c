//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define LINE_LENGTH 81

typedef struct Linked_List{
    char instruction[LINE_LENGTH];
    struct Linked_List* next_instruction;
} Linked_List;

typedef struct  macro_Linked_list{
    char name[30];
    struct Linked_List* first_instruction;
    struct macro_Linked_list* next_macro;
}macro_Linked_list;

int contains(macro_Linked_list* macro_table, char* command_name) {
    if (command_name[strlen(command_name) - 1] == '\n') {
        command_name[strlen(command_name) - 1] = '\0'; // remove newline if there is any
    }
    macro_Linked_list* iteratepoint = macro_table;
    while (iteratepoint != NULL) {
        if (strcmp(command_name, iteratepoint->name) == 0) {
            return 1;
        }
        macro_Linked_list* trmp = iteratepoint->next_macro;
        iteratepoint = trmp;
    }
    return 0;
}

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
    for (int i = 0; name[i] != '\0' && name[i] != '\n'; i++) {
        if (isalnum(name[i]) == 0 && name[i] != '_') {
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
    char line[LINE_LENGTH];
    macro_Linked_list* macro_table = malloc(sizeof(macro_Linked_list));
    if (!macro_table) {
        fprintf(stderr, "not memory\n");
        exit(1);
    }
    macro_table->first_instruction = NULL;
    macro_table->next_macro = NULL;
    strcpy(macro_table->name, "");
    // pointer to the begging of the table to "save" the original pointer;
    macro_Linked_list* mcropoint = macro_table;
    Linked_List* instructionpoint;
    int exists_mcro = 0;

    while (fgets(line, LINE_LENGTH, source_asm)) {
        // check if line is longer than the limit
        if (strstr(line, "\n") == NULL) { // reached end of fgets count amount of chars without end line.
            if (fgetc(source_asm) != EOF) { // when last line doesn't have "\n"
                fprintf(stderr, "Error in line %d. The instruction is longer than 80 chars.\n", IC);
                exit(1);
            }
        }

        int i = 0;
        for (i = 0; line[i] == ' ' || (int)line[i] == '\t'; i++) {} // advance i to the first non tab or space char
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, &line[i],LINE_LENGTH);

        char *first_word = strtok(line_for_tokenisation, " ");
        char *second_word = strtok(NULL, " \t\n");
        char *third_word = strtok(NULL, " \t\n");
        // this isn't such a good way to parse the instructions
        // in the actual not in the pre i will do it differently
        // check if first word of the instruction is mcro
        // check if "third" word is null (meaning the instruction is mcro {name}\n or '\0')
        // handle space after name (is ok)

        if (strcmp(first_word, "mcro") == 0 && (third_word == NULL || third_word[0] == '\n')) { //
            if (name_valid(second_word, IC) == 0) {
                exit(1);
            }
            exists_mcro = 1;
            strcpy(mcropoint->name, second_word);
            mcropoint->first_instruction = NULL;
            macro_Linked_list *to_add = malloc(sizeof (macro_Linked_list));
            to_add->next_macro = NULL;
            to_add->first_instruction = NULL;
            mcropoint->next_macro = to_add;;
            instructionpoint = mcropoint->first_instruction; // pointer to add instruction too

        }
        else if (strcmp(first_word, "mcroend\n") == 0) {
            exists_mcro = 0;
            mcropoint = mcropoint->next_macro;
        }
        else if (exists_mcro) {
            Linked_List *to_add = malloc(sizeof (Linked_List));
            if (!to_add) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            strcpy(to_add->instruction, line);
            to_add->next_instruction = NULL;
            if (mcropoint->first_instruction == NULL) {
                mcropoint->first_instruction = to_add;
            }
            else {
                instructionpoint->next_instruction = to_add;
            }
            instructionpoint = to_add;
        }
        else { // comes here if (exists mcro = 0, the command is not mcro and not end mcro meaning it's instruction
            macro_Linked_list* banana = macro_table;

            if (contains(banana, first_word)) {
                // bring the macro hereee
                macro_Linked_list* iteratepoint = macro_table;
                if (first_word[strlen(first_word) - 1] == '\n') {
                    first_word[strlen(first_word) - 1] = '\0'; // remove newline if there is any
                }
                while (iteratepoint != NULL) {
                    if (strcmp(iteratepoint->name, first_word) == 0) {
                        Linked_List* iterate_linked = iteratepoint->first_instruction;
                        while (iterate_linked!=NULL) {
                            fprintf(post_pre_asm, iterate_linked->instruction);
                            Linked_List* temp = iterate_linked->next_instruction;
                            iterate_linked = temp;
                        }
                    }
                    macro_Linked_list* trmp = iteratepoint->next_macro;
                    iteratepoint = trmp;
                }
            }
            else {
                fprintf(post_pre_asm, line);
                printf("%s", line);
            }
        }
        IC++;
    }

    // free my boy :)
    macro_Linked_list* iteratepoint = macro_table;
    while (iteratepoint != NULL) {
        Linked_List* iterate_linked = iteratepoint->first_instruction;
        while (iterate_linked!=NULL) {
            Linked_List* temp = iterate_linked->next_instruction;
            free(iterate_linked);
            iterate_linked = temp;
        }
        macro_Linked_list* trmp = iteratepoint->next_macro;
        free(iteratepoint);
        iteratepoint = trmp;
    }

    // finished making the macro table

    fclose(source_asm);
    fclose(post_pre_asm);
    exit(9);
}