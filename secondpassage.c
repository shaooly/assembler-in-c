//
// Created by Shauli on 11/07/2025.
//

#include "secondpassage.h"
#include "firstpassage.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include "secondpassage.h"
#include "firstpassage.h"

#define LABEL_PADDING 2
#define R_BIT 1



// checks if in label list if yes returns the label object associated with it in the list
label_list *in_label_list(label_list *list, char *label) {
    label_list *tmp = list;
    while (tmp != NULL) {
        if (strcmp(tmp->label_name, label) == 0) {
            return tmp;
        }
        tmp = tmp->next_label;
    }
    return NULL;
}

// takes in LC as an input and returns the according binary_line object representation
binary_line *get_binay_line(binary_line *line_list, int LC) {
    binary_line *tmp = line_list;
    while (tmp != NULL) {
        if (tmp->LC == LC) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

void build_label_word(label_list *the_label_list, binary_line *current_line, char *label, int address_pos, int LC, int *error) {
    label_list *label_object = in_label_list(the_label_list, label);
    if (label_object != NULL) { // we found the label in the list !
        if (strcmp(label_object->label_type, "external") != 0) {
            // shift the location and OR with A,R,E bit (01)
            unsigned short label_word = (label_object->value << LABEL_PADDING) | R_BIT;
            current_line->words[current_line->labels_addressing[address_pos]] = label_word;
        }
    }
    else {
        fprintf(stderr, "Error in line %d. label not in list.\n", LC); // check
        *error = 1;
    }
}


int second_passage(binary_line *binary_line_list, label_list *the_label_list, int DCF, int ICF) {
    int exists_error = 0;
    int LC = 1; // we will match it with LC's !
    FILE *source_asm = fopen("postpre.asm", "r");

    char line[LINE_LENGTH];

    while (fgets(line, LINE_LENGTH, source_asm)) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        char *second_word = 0;
        char *first_word = strtok(line_for_tokenisation, " \t\n");
        char *third_word = 0;

        if (first_word == NULL) { // empty line - skip
            LC++;
            continue;
        }
        if (first_word[strlen(first_word) - 1] == ':') { // label // 2
            first_word = strtok(NULL, " \t\n"); // ignoring the label
        }
        if (strcmp(first_word, ".data") == 0
            || strcmp(first_word, ".string") == 0
            || strcmp(first_word, ".mat") == 0
            || strcmp(first_word, ".extern") == 0) { // 3
            LC++;
            continue;
            }
        second_word = strtok(NULL, " ,\t\n");
        third_word = strtok(NULL, " ,\t\n");
        // if (third_word != NULL) {
        //     fprintf(stderr, "Error in line %d. More declarations after entry is bad.\n", LC); // check
        //     exists_error = 1;
        //     return 0;
        // }
        if (strcmp(first_word, ".entry") == 0) { // 4
            if (in_label_list(the_label_list, second_word) != NULL) {
                label_list *tmp = the_label_list;
                while (tmp != NULL) {
                    if (strcmp(tmp->label_name, second_word) == 0) {
                        strcat(tmp->label_type, "entry"); // 5
                    }
                    tmp = tmp->next_label;
                }
            }
            else {
                fprintf(stderr, "Error in line %d. entry label not in list.\n", LC, second_word); // check
                exists_error = 1;
            }
        }
        //
        else { // 6
            binary_line *current_line = get_binay_line(binary_line_list, LC);
            // if (current_line == NULL) {
            //     fprintf(stderr, "Error in line %d. idk what happened.\n", LC);
            // }
            char *source_label = current_line->labels[0];
            char *destination_label = current_line->labels[1];
            if (source_label != "") { // we have a source label!
                build_label_word(the_label_list, current_line, source_label, 0, LC, &exists_error);
            }
            if (destination_label != "") { // we have a destination label!
                build_label_word(the_label_list, current_line, destination_label, 1, LC, &exists_error);
            }
        }

        LC++;
    }
    if (exists_error) { // 7
        free_all(the_label_list, binary_line_list);
        fclose(source_asm);
        exit(1);
    }
    free_all(the_label_list, binary_line_list);
    // build_source_files // 8
    return 0;
}
