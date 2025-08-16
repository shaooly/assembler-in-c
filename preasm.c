/*
 * The pre-assembler.
 * This code handles all the macro de-macroing.
 * The code basically scans the entire code and searches for this pattern:
 *
 * mcro {name}
 * --code--
 * mcroend
 *
 * Then, it searches for the name and switches the name if the entire code inside the macro.
 * When creating the file post the pre assembler, it removes all the macro definitions and switches the name out
 * for the actual macro definitions
 *
 * Author: Shaul Joseph Sasson
 * Created: 30/05/2025
 * Depends on preasm.h, firstpassage.h
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "preasm.h"
#include "firstpassage.h"

/*
 * Checks if a (technically "the") macro table contains the name of the macro.
 * Params:
 *      macro_table - pointer to the macro linked list
 *      command_name - the name of the macro we want to check
 * Returns: 1 if found, 0 if not found
 *
 */
int contains(macro_Linked_list* macro_table, char* command_name) {
    macro_Linked_list* iteratepoint;
    if (command_name == NULL) {
        return 0;
    }
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

/*
 * Validates a macro name by the rules.
 * Params: name - the candidate macro name
 *         IC   - current line number (for error reporting)
 * Returns: 1 if valid, 0 otherwise
 */

int name_valid(char *name, int IC) {
    int i =0;
    const char *known_instructions[INSTRUCTION_AMOUNT] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend"};

    if (strlen(name) > MAX_MACRO_LEN) {
        fprintf(stderr, "Error in line %d: The macro name is longer than 30 chars.\n", IC);
        return 0;
    }
    if (isalpha(name[0]) == 0) {
        fprintf(stderr, "Error in line %d: The first char in macro name is a number.\n", IC);
        return 0;
    }
    for (i = 0; name[i] != '\0' && name[i] != '\n'; i++) {
        if (isalnum(name[i]) == 0 && name[i] != '_') {
            fprintf(stderr, "Error in line %d: The char in the %dth position in macro name is not alphanumeric.\n"
                , IC, i);
            return 0;
        }
    }
    // amud 28 lahzor
    if (name[strlen(name) - 1] == '\n') {
        name[strlen(name) - 1] = '\0'; // remove newline if there is any
    }
    for (i = 0; i < INSTRUCTION_AMOUNT; i++) {
        if (strcmp(name, known_instructions[i]) == 0) { // if macro name is known instruction name.
            fprintf(stderr, "Error in line %d: The macro name is a known instruction name (%s).", IC,
                known_instructions[i]);
            return 0;
        }
    }
    return 1;
}

/*
 * Takes an instruction (technically the entire line but called instruction) and inserts it into the macro_table
 * in the current place it's on (since we advance it with each iteration)
 */

static Linked_List* insert_instruction(macro_Linked_list *macro_table, Linked_List *previous, char *line,
    int *exists_error, int IC) {
    Linked_List *to_add = malloc(sizeof (Linked_List));
    memset(to_add, 0, sizeof (Linked_List));
    if (!to_add) {
        *exists_error = 1;
        fprintf(stderr, "Error in line %d: Memory allocation failed\n", IC);
        return previous;
    }
    strcpy(to_add->instruction, line);
    to_add->next_instruction = NULL;
    if (macro_table->first_instruction == NULL) {
        macro_table->first_instruction = to_add;
    }
    else {
        previous->next_instruction = to_add;
    }
    return to_add;
}

/*
 * The actual pre-assembler stage.
 * Params: filename      - input source (.as)
 *         post_filename - output file with macros expanded
 * Returns: pointer to the macro table.
 */

macro_Linked_list* pre_asm(char *filename, char *post_filename){
    int exists_error = 0;
    int IC = 1;
    Linked_List* instructionpoint = NULL;
    char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
    char *first_word;
    char *second_word;
    char *third_word;
    char line[LINE_LENGTH];
    int exists_mcro;
    FILE *post_pre_asm;
    FILE *source_asm = fopen(filename, "r");
    if (!source_asm) {
        fprintf(stderr, "Error: Couldn't open the file %s", filename);
        return NULL;
    }
    post_pre_asm = fopen(post_filename, "w");
    if (!post_pre_asm) {
        fprintf(stderr, "Error: Couldn't open the file %s", post_filename);
        return NULL;
    }

    macro_Linked_list* mcropoint;
    macro_Linked_list* macro_table = malloc(sizeof(macro_Linked_list));
    if (!macro_table) {
        fprintf(stderr, "Error in line %d: not enough memory to malloc.\n", IC);
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
        if (line[LINE_LENGTH - 1] != '\0' && line[strlen(line) - 1] != '\n') {
            if (fgetc(source_asm) != EOF) { // when last line doesn't have "\n"
                fprintf(stderr, "Error in line %d: The instruction is longer than 80 chars.\n", IC);
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
        third_word = NULL;
        if (second_word != NULL) {
            third_word = strtok(NULL, " \t\n");
        }
        /*  this isn't such a good way to parse the instructions in my opinion but i basically try to make sure
            it is in the format of mcro {name} {nothing more here}
            in order to do that i do a couple of checks:
            check if first word of the instruction is mcro
            check if "third" word is null (meaning the instruction is mcro {name}\n or '\0')
            handle space after name (is ok) */
        if (strcmp(first_word, "mcro") == 0 && (third_word == NULL || third_word[0] == '\n')) { //
            macro_Linked_list *to_add;
            if (second_word == NULL) {
                fprintf(stderr, "Error in line %d: defined a mcro with no name.", IC);
                exists_error = 1;
                break;
            }
            if (name_valid(second_word, IC) == 0) {
                exists_error = 1;
            }
            else if (contains(macro_table, second_word)) {
                fprintf(stderr, "Error in line %d: duplicate macro name %s. \n", IC, second_word);
                exists_error = 1;
            }
            else {
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
            }
        }
        else if (strcmp(first_word, "mcroend") == 0 || strcmp(first_word, "mcroend\n") == 0) {
            if (mcropoint->first_instruction == NULL) {
                fprintf(stderr, "Error in line %d: empty macro is illegal", IC);
                exists_error = 1;
            }
            exists_mcro = 0;
            mcropoint = mcropoint->next_macro;
        }
        else if (exists_mcro) {
            instructionpoint = insert_instruction(mcropoint, instructionpoint, line, &exists_error, IC);
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
    /*
     * If an error exists, there's no point in creating the post file.
     * So we're freeing the macro linked list so it won't leak and closing the program
     */
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
        fclose(source_asm);
        fclose(post_pre_asm);
        fflush(stderr);
        return NULL;
    }
    fclose(source_asm);
    fclose(post_pre_asm);
    return macro_table;
}
