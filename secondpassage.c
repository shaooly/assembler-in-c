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
#define R_BIT 2



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

char *make_yihoodi_number(int num, int size) {
    char *base_4_special_number = malloc(1 + size);
    int i;
    if (num < 0) {
        num = (int) pow(4, size) - abs(num); // account for negative numbers
    }

    for (i = 0; i < size; i++) {
        int times_fit = floor(num / pow(4, size - 1 - i));
        char letter_in_i_pos = times_fit + 'a';
        base_4_special_number[i] = letter_in_i_pos;
        num = num - (int) pow(4, size - 1 - i) * times_fit;

    }
    base_4_special_number[size] = '\0';
    return base_4_special_number;
}

void build_label_word(label_list *the_label_list, binary_line *current_line, char *label, int address_pos, int LC, int *error) {
    label_list *label_object = in_label_list(the_label_list, label);
    if (label_object != NULL) { // we found the label in the list !
        if (strcmp(label_object->label_type, "external") != 0) {
            // shift the location and OR with A,R,E bit (10)
            unsigned short label_word = (label_object->value << LABEL_PADDING) | R_BIT;
            int position = current_line->labels_addressing[address_pos];
            if (position != 0) {
                current_line->words[position] = label_word;
            }
            //printf("words after changing addressing : %d\n", current_line->words[0]);
        }
        else { // if external
            unsigned short label_word = 1; // zero's and 01 at the end according to E bitting
            int position = current_line->labels_addressing[address_pos];
            if (position != 0) {
                current_line->words[position] = label_word;
            }
            FILE *ext_file = fopen("ps.ext", "a");
            char *label_name = label_object->label_name;
            // i broke my head on the line below
            // honestly, idk why but it was hard for me to get it
            char *base4_address = make_yihoodi_number(current_line->IC + position, 4);
            fprintf(ext_file, label_name);
            fprintf(ext_file, "\t");
            fprintf(ext_file, base4_address);
            fprintf(ext_file, "\n");
            free(base4_address);
            fclose(ext_file);
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
                fprintf(stderr, "Error in line %d. entry label not in list.\n", LC); // check
                exists_error = 1;
            }
        }
        //
        else {
            // 6

            binary_line *current_line = get_binay_line(binary_line_list, LC);

            if (current_line == NULL) {
                fprintf(stderr, "Error in line %d. idk what happened.\n", LC);
            }
            else {
                char *source_label = current_line->labels[0];

                char *destination_label = current_line->labels[1];
                if (source_label != "") { // we have a source label!
                    build_label_word(the_label_list, current_line, source_label, 0, LC, &exists_error);
                }
                if (destination_label != "") { // we have a destination label!
                    build_label_word(the_label_list, current_line, destination_label, 1, LC, &exists_error);
                }
            }
        }
        LC++;
    }
    if (exists_error) { // 7

        free_all(the_label_list, binary_line_list);
        fclose(source_asm);
        exit(1);
    }


    // build source files

    FILE *object_file = fopen("file.obj", "w");
    binary_line *tmp1 = binary_line_list;

    while (tmp1 != NULL) {
        int i;
        for (i = 0; i<5; i++) {
            if (tmp1->words[i] != 0) {
                char *base4_instruction = make_yihoodi_number(tmp1->words[i], 5);
                char *base4_address = make_yihoodi_number(tmp1->IC + i, 4);
                if (strcmp(base4_instruction, "aaaab") == 0) { // the only way the instruction is aaaab is if ext

                }
                fprintf(object_file, base4_address);
                fprintf(object_file, "\t");
                fprintf(object_file, base4_instruction);
                fprintf(object_file, "\n");
                free(base4_address);
                free(base4_instruction);
            }
        }
        tmp1 = tmp1->next;
    }
    int DC = ICF;
    int i;
    for (i = 0; i < memory_pointer; i++) {
        char *label = make_yihoodi_number(memory[i], 5);
        char *base4_address = make_yihoodi_number(DC + i, 4);
        fprintf(object_file, base4_address);
        fprintf(object_file, "\t");
        fprintf(object_file, label);
        fprintf(object_file, "\n");
        free(label);
        free(base4_address);
    }
    fclose(object_file);



    label_list *tmp = the_label_list;
    // iterate label list and extract entries
    int exists_entry = 0; // flag if entry exists and create file.
    // i know it would've been possible to do it in the same loop but i want readable code and it doesn't save
    // that much space
    while (tmp != NULL) {
        // strstr checks if "entry" is a substring
        if (strstr(tmp->label_type, "entry")) {
            printf("hi i'm %s and i'm entry", tmp->label_name);
            exists_entry = 1;
        }
        tmp = tmp->next_label;
    }
    if (exists_entry) {
        FILE *entry_file = fopen("ps.ent", "w");
        label_list *tmp2 = the_label_list;
        while (tmp2 != NULL) {
            if (strstr(tmp2->label_type, "entry")) {
                fprintf(entry_file, tmp2->label_name);
                fprintf(entry_file, "\t");
                char *base4_address = make_yihoodi_number(tmp2->value, 4);
                fprintf(entry_file, base4_address);
                fprintf(entry_file, "\n");
                free(base4_address);
            }
            tmp2 = tmp2->next_label;
        }

    }

    free_all(the_label_list, binary_line_list);
    printf("finsihed all");
    // build_source_files // 8
    return 0;
}
