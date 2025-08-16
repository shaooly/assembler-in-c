/*
 * The second passage of the assembler.
 * The passage scans the file again, completes the label addressing and creates the ouput files
 *
 * Author: Shaul Joseph Sasson
 * Created 11/07/2025.
 *
 * Depends on firstpassage.h, memory.h, secondpassage.h
 */
#include "secondpassage.h"
#include "firstpassage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"


#define LABEL_PADDING 2
#define R_BIT 2



/* defining my own int_pow because i want it to compile beautifully with 0 errors
 * not that its hard i'm just explaining why i'm doing it
 */
int my_pow (int a, int b) {
    int result = 1;
    int i;
    for (i = 1; i <= b; i++) {
        result *= a;
    }
    return result;
}

/* defining my own int_pow because i want it to compile beautifully with 0 errors
 * not that its hard i'm just explaining why i'm doing it
 */
int my_floor(double a) {
    int result = (int)a;
    return result;

}

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
binary_line *get_binary_line(binary_line *line_list, int LC) {
    binary_line *tmp = line_list;
    while (tmp != NULL) {
        if (tmp->LC == LC) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

// creates the number in the specified base like in the hoveret
// basically bitwise ops
// i am doing this good because i studied IRGOON with you Rachmani and now i'm the pro in base shifting
char *make_yihoodi_number(int num, int size, int *error) {
    char *base_4_special_number = malloc(1 + size);
    int i;
    if (!base_4_special_number) {
        *error = 1;
        fprintf(stderr, "Error on host computer: failed to malloc.");
        return NULL;
    }
    if (num < 0) {
        num = my_pow(4, size) - abs(num); // account for negative numbers
    }
    for (i = 0; i < size; i++) {
        int times_fit = my_floor(num / my_pow(4, size - 1 - i));
        char letter_in_i_pos = times_fit + 'a';
        base_4_special_number[i] = letter_in_i_pos;
        num = num - my_pow(4, size - 1 - i) * times_fit;

    }
    base_4_special_number[size] = '\0';
    return base_4_special_number;
}

/*
 * This function basically "fills the holes" we created in the first passage for the binary lines
 * This is where to struct we defined earlier with the labels comes super handy.
 * All we need to do is access the corresponding label for the line and if found and it inserts it into place.
 *
 * Raises an error if label is not found.
 */
void build_label_word(label_list *the_label_list, binary_line *current_line, char *label, int address_pos, int LC,
    int *error, char *original_argv) {
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
            char *label_name;
            FILE *ext_file;
            int position = current_line->labels_addressing[address_pos];
            char *external_file_name;
            char *base4_address;
            size_t len;
            if (position != 0) {
                current_line->words[position] = label_word;
            }
            len = strlen(original_argv);
            external_file_name = malloc(len + 5); /* + 4 for the .ext */
            strcpy(external_file_name, original_argv);
            strcat(external_file_name, ".ext");
            ext_file = fopen(external_file_name, "a");
            free(external_file_name);
            if (!ext_file) {
                *error = 1;
                fprintf(stderr, "Error on host computer: couldn't open the external file\n");
                return;
            }
            label_name = label_object->label_name;
            // i broke my head on the line below
            // honestly, idk why but it was hard for me to get it
            base4_address = make_yihoodi_number(current_line->IC + position, 4, error);
            if (base4_address != NULL) {
                fprintf(ext_file, "%s", label_name);
                fprintf(ext_file, "\t");
                fprintf(ext_file, "%s", base4_address);
                fprintf(ext_file, "\n");
                free(base4_address);
            }
            fclose(ext_file);
        }
    }
    else {
        fprintf(stderr, "Error in line %d: label not in list.\n", LC); // check
        *error = 1;
    }
}




void second_passage(binary_line *binary_line_list, label_list *the_label_list, int ICF, int DCF,
    macro_Linked_list *macro_table, char *file_name, char *original_argv) {
    int exists_error = 0;
    int LC = 1; // we will match it with LC's !
    FILE *source_asm = fopen(file_name, "r");
    FILE *object_file;
    char line[LINE_LENGTH];
    int i;
    int DC;
    label_list *tmp;
    binary_line *tmp1;
    int exists_entry;
    size_t len;
    char *object_file_name;
    char *base4_ICF;
    char *base4_DCF;
    if (!source_asm) {
        fprintf(stderr, "Error on host computer: couldn't open the source post pre file\n");
        free_all(the_label_list, binary_line_list, macro_table);
        return;
    }
    while (fgets(line, LINE_LENGTH, source_asm)) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        char *second_word = 0;
        char *first_word;
        // char *third_word = 0;
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        line_for_tokenisation[LINE_LENGTH-1] = '\0';
        first_word = strtok(line_for_tokenisation, " \t\n");

        if (first_word == NULL) { // empty line
            LC++;
            continue;
        }
        if (first_word[0] == ';') { // comment
            LC++;
            continue;
        }

        if (first_word[strlen(first_word) - 1] == ':') { // label // 2
            first_word = strtok(NULL, " \t\n"); // ignoring the label
        }
        if (first_word == NULL) {
            LC++;
            continue;
        }
        if (strcmp(first_word, ".data") == 0 || strcmp(first_word, ".string") == 0 ||
            strcmp(first_word, ".mat") == 0 || strcmp(first_word, ".extern") == 0) { // 3
            LC++;
            continue;
            }
        second_word = strtok(NULL, " ,\t\n");
        if (strcmp(first_word, ".entry") == 0) { // 4
            if (second_word != NULL && in_label_list(the_label_list, second_word) != NULL) {
                label_list *tmp2 = the_label_list;
                while (tmp2 != NULL) {
                    if (strcmp(tmp2->label_name, second_word) == 0) {
                        strcat(tmp2->label_type, "entry"); // 5
                    }
                    tmp2 = tmp2->next_label;
                }
            }
            else {
                fprintf(stderr, "Error in line %d: entry label not in list.\n", LC);
                exists_error = 1;
            }
        }
        //
        else {
            // 6
            binary_line *current_line = get_binary_line(binary_line_list, LC);
            if (current_line == NULL) {
                exists_error = 1;
                fprintf(stderr, "Error in line %d: line not found.\n", LC);
            }
            else {
                char *source_label = current_line->labels[0];

                char *destination_label = current_line->labels[1];
                if (source_label != NULL && strcmp(source_label, "") != 0) { // we have a source label!
                    build_label_word(the_label_list, current_line, source_label, 0, LC, &exists_error, original_argv);
                }
                if (destination_label != NULL && strcmp(destination_label, "") != 0) { // we have a destination label!
                    build_label_word(the_label_list, current_line, destination_label, 1, LC, &exists_error, original_argv);
                }
            }
        }
        LC++;
    }
    if (exists_error) { // 7
        size_t size;
        char *external_file_name;
        size = strlen(original_argv);
        external_file_name = malloc(size + 5); /* 5 for the .ext + '\0' */
        strcpy(external_file_name, original_argv);
        strcat(external_file_name, ".ext");
        remove(external_file_name);
        free(external_file_name);

        free_all(the_label_list, binary_line_list, macro_table);
        fclose(source_asm);
        return;
    }

    // build source files

    len = strlen(original_argv);

    object_file_name = malloc(len + 4); /* + 4 for the .ob */
    strcpy(object_file_name, original_argv);
    strcat(object_file_name, ".ob");
    object_file = fopen(object_file_name, "w");
    free(object_file_name);
    if (!object_file) {
        exists_error = 1;
        fprintf(stderr, "Error on host computer: Couldn't open the object file.\n");
        free_all(the_label_list, binary_line_list, macro_table);
        fclose(source_asm);
        return;
    }

    /* Add DCF and ICF */
    base4_ICF = make_yihoodi_number(ICF - 100, 4, &exists_error); /* -100 because we started IC at 100 */
    base4_DCF = make_yihoodi_number(DCF, 4, &exists_error);
    if (base4_ICF != NULL) {
        fprintf(object_file, "%s", base4_ICF);
        fprintf(object_file, "\t");
        free(base4_ICF);
    }
    if (base4_DCF != NULL) {
        fprintf(object_file, "%s", base4_DCF);
        fprintf(object_file, "\n");
        free(base4_DCF);
    }

    tmp1 = binary_line_list;
    while (tmp1 != NULL) {
        int j;
        for (j = 0; j< tmp1->L; j++) {
            char *base4_instruction = make_yihoodi_number(tmp1->words[j], 5, &exists_error);
            char *base4_address = make_yihoodi_number(tmp1->IC + j, 4, &exists_error);
            if (base4_address != NULL) {
                fprintf(object_file, "%s", base4_address);
                fprintf(object_file, "\t");
                free(base4_address);
            }
            if (base4_instruction != NULL) {
                fprintf(object_file, "%s", base4_instruction);
                fprintf(object_file, "\n");
                free(base4_instruction);
            }
        }
        tmp1 = tmp1->next;
    }


    DC = ICF;

    for (i = 0; i < memory_pointer; i++) {
        char *label = make_yihoodi_number(memory[i], 5, &exists_error);
        char *base4_address = make_yihoodi_number(DC + i, 4, &exists_error);
        if (base4_address != NULL) {
            fprintf(object_file, "%s", base4_address);
            fprintf(object_file, "\t");
            free(base4_address);
        }
        if (label != NULL) {
            fprintf(object_file, "%s", label);
            fprintf(object_file, "\n");
            free(label);
        }

    }
    fclose(object_file);
    tmp = the_label_list;
    // iterate label list and extract entries
    exists_entry = 0; // flag if entry exists and create file.
    // i know it would've been possible to do it in the same loop but i want readable code and it doesn't save
    // that much space
    while (tmp != NULL) {
        // strstr checks if "entry" is a substring
        if (strstr(tmp->label_type, "entry")) {
            exists_entry = 1;
        }
        tmp = tmp->next_label;
    }
    if (exists_entry) {
        size_t size = strlen(original_argv);
        FILE *entry_file;
        char *entry_file_name = malloc(size + 5); /* + 5 for the .ent + \0 */
        label_list *tmp2;
        strcpy(entry_file_name, original_argv);
        strcat(entry_file_name, ".ent");
        entry_file = fopen(entry_file_name, "w");
        free(entry_file_name);
        if (!entry_file) {
            free_all(the_label_list, binary_line_list, macro_table);
            fclose(source_asm);
            exists_error = 1;
            fprintf(stderr, "Error on host computer: Couldn't open the entry file.\n");
            return;
        }
        tmp2 = the_label_list;
        while (tmp2 != NULL) {
            if (strstr(tmp2->label_type, "entry")) {
                char *base4_address = make_yihoodi_number(tmp2->value, 4, &exists_error);
                if (base4_address != NULL) {
                    fprintf(entry_file, "%s", tmp2->label_name);
                    fprintf(entry_file, "\t");
                    fprintf(entry_file, "%s", base4_address);
                    fprintf(entry_file, "\n");
                    free(base4_address);
                }
            }
            tmp2 = tmp2->next_label;
        }
        fclose(entry_file);

    }
    free_all(the_label_list, binary_line_list, macro_table);
    fclose(source_asm);
    // build_source_files - 8
}
