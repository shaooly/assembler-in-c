//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "preasm.h"
#include "firstpassage.h"

int contains(macro_Linked_list* macro_table, char* command_name) {
    macro_Linked_list* iteratepoint;
    if (command_name[strlen(command_name) - 1] == '\n') {
        command_name[strlen(command_name) - 1] = '\0'; // remove newline if there is any
    }
    iteratepoint = macro_table;
    while (iteratepoint != NULL) {
        macro_Linked_list* trmp;
        if (strcmp(command_name, iteratepoint->name) == 0) {
            return 1;
        }
        trmp = iteratepoint->next_macro;
        iteratepoint = trmp;
    }
    return 0;
}

int name_valid(char *name, int IC) {
    int i =0;
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
    for (i = 0; name[i] != '\0' && name[i] != '\n'; i++) {
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
    for (i = 0; i < 18; i++) {
        if (strcmp(name, known_instructions[i]) == 0) { // if macro name is known instruction name.
            fprintf(stderr, "Error in line %d. The macro name is a known instruction name (%s).", IC,
                known_instructions[i]);
            return 0;
        }
    }
    return 1;
}


macro_Linked_list* pre_asm(char *filename, char *post_filename){
    int exists_error = 0;
    int IC = 1;
    Linked_List* instructionpoint;
    char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
    char *first_word;
    char *second_word;
    char *third_word;
    int exists_mcro;
    FILE *source_asm = fopen(filename, "r");
    FILE *post_pre_asm = fopen(post_filename, "w");
    char line[LINE_LENGTH];
    int i;
    macro_Linked_list* mcropoint;
    macro_Linked_list* macro_table = malloc(sizeof(macro_Linked_list));
    if (!macro_table) {
        fprintf(stderr, "not memory\n");
        exit(1);
    }
    memset(macro_table, 0, sizeof(macro_Linked_list)); /* valgrind yelling */
    macro_table->first_instruction = NULL;
    macro_table->next_macro = NULL;
    strcpy(macro_table->name, "");
    // pointer to the begging of the table to "save" the original pointer;
    mcropoint = macro_table;
    exists_mcro = 0;
    while (fgets(line, LINE_LENGTH, source_asm)) {
        // check if line is longer than the limit
        if (strstr(line, "\n") == NULL) { // reached end of fgets count amount of chars without end line.
            if (fgetc(source_asm) != EOF) { // when last line doesn't have "\n"
                fprintf(stderr, "Error in line %d. The instruction is longer than 80 chars.\n", IC);
                exists_error = 1;
            }
        }
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        first_word = strtok(line_for_tokenisation, " \t\n");
        if (first_word == NULL) { // empty line
            IC++;
            continue;
        }
        if (first_word[0] == ';') { // comment
            IC++;
            continue;
        }

        second_word = strtok(NULL, " \t\n");
        third_word = strtok(NULL, " \t\n");
        // this isn't such a good way to parse the instructions
        // in the actual not in the pre i will do it differently
        // check if first word of the instruction is mcro
        // check if "third" word is null (meaning the instruction is mcro {name}\n or '\0')
        // handle space after name (is ok)
        if (strcmp(first_word, "mcro") == 0 && (third_word == NULL || third_word[0] == '\n')) { //
            macro_Linked_list *to_add;
            if (name_valid(second_word, IC) == 0) {
                exists_error = 1;
            }
            exists_mcro = 1;
            strcpy(mcropoint->name, second_word);
            mcropoint->first_instruction = NULL;
            to_add = malloc(sizeof (macro_Linked_list));
            /* i actually prefer to set everything to null
             * and i've been doing so for years coding in java
             * but valgrind is screaming so i'm using the memset alternative hopefully it will work
             */
            memset(to_add, 0, sizeof (macro_Linked_list));
            mcropoint->next_macro = to_add;
            instructionpoint = mcropoint->first_instruction; // pointer to add instruction too
            printf("adding macro with name: %s", second_word);
        }
        else if (strcmp(first_word, "mcroend\n") == 0) {
            exists_mcro = 0;
            mcropoint = mcropoint->next_macro;
        }
        else if (exists_mcro) {
            Linked_List *to_add = malloc(sizeof (Linked_List));
            memset(to_add, 0, sizeof (Linked_List));
            if (!to_add) {
                exists_error = 1;
                fprintf(stderr, "Memory allocation failed\n");
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
            if (contains(banana, first_word) || (first_word[strlen(first_word) - 1] == ':'
                        && contains(banana, second_word)
                        && third_word == NULL)) { // check if command is in macro table
                // bring the macro hereee
                macro_Linked_list* iteratepoint = macro_table;
                if (first_word[strlen(first_word) - 1] == '\n') {
                    first_word[strlen(first_word) - 1] = '\0'; // remove newline if there is any
                }
                while (iteratepoint->next_macro != NULL) {
                    macro_Linked_list* trmp;
                    if (strcmp(iteratepoint->name, first_word) == 0) {
                        Linked_List* iterate_linked = iteratepoint->first_instruction;
                        Linked_List* temp;
                        while (iterate_linked!=NULL) {
                            fprintf(post_pre_asm, "%s", iterate_linked->instruction);
                            temp = iterate_linked->next_instruction;
                            iterate_linked = temp;
                        }
                    }
                    if (second_word != NULL) {
                        if (first_word[strlen(first_word) - 1] == ':'
                            && strcmp(iteratepoint->name, second_word) == 0
                            && third_word == NULL) {
                            Linked_List* iterate_linked = iteratepoint->first_instruction;
                            fprintf(post_pre_asm, "%s", first_word);
                            while (iterate_linked!=NULL) {
                                Linked_List* temp;
                                fprintf(post_pre_asm, "%s", iterate_linked->instruction);
                                temp = iterate_linked->next_instruction;
                                iterate_linked = temp;
                            }
                        }
                    }
                    trmp = iteratepoint->next_macro;
                    iteratepoint = trmp;
                }
            }
            else {
                fprintf(post_pre_asm, "%s", line);
            }
        }
        IC++;
    }
    // free my boy :)
    if (exists_error) {
        macro_Linked_list* trmp;
        macro_Linked_list* iteratepoint = macro_table;
        while (iteratepoint->first_instruction != NULL) {
            Linked_List* iterate_linked = iteratepoint->first_instruction;
            while (iterate_linked!=NULL) {
                Linked_List* temp = iterate_linked->next_instruction;
                free(iterate_linked);
                iterate_linked = temp;
            }
            trmp = iteratepoint->next_macro;
            free(iteratepoint);
            iteratepoint = trmp;
        }
        free(iteratepoint); // free the final "NULL" node malloced to null pointers :)
        exit(1);
    }
    // leaked so much memory before this haha
    // finished making the macro table
    fclose(source_asm);
    fclose(post_pre_asm);
    return macro_table;
}

int memory_pointer = 0;  // define memory pointer probably will change it later
int memory[256] = {0};

int main(int argc, char *argv[]) {
    int j;
    size_t len;
    macro_Linked_list *macro_table;
    char *file_name;
    char *post_file_name;
    for (j = 1; j<argc; j++) {
        int w;
        for (w = 0; w < 256; w++) {
            memory[w] = 0;
        }
        memory_pointer = 0;
        printf("handling %s\n", argv[j]);

        len = 4 + strlen(argv[j]); /* using size_t because of conversion stuff and you know */
        file_name = malloc(len);

        /* this is responsible for opening the original file*/
        strcpy(file_name, argv[j]);
        strcat(file_name, ".as");

        /* this makes the file post the macros spread */
        post_file_name = malloc(len + 5); /* the 4 is for the word post */
        strcpy(post_file_name, "post");
        strcat(post_file_name, file_name);

        /* post_file_name = post{ogfilename}.as */
        macro_table = pre_asm(file_name, post_file_name);

        /* pass the original argv because i don't know how to strip the "post" and ".as"*/
        first_passage(macro_table, post_file_name, argv[j]);
        free(post_file_name);
        free(file_name);
    }
    return 0;
}