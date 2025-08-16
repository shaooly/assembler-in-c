/*
 * Header file for the first passage.
 * Author: Shaul Joseph Sasson
 */
#ifndef FIRSTPASSAGE_H
#define FIRSTPASSAGE_H

#define MAX_LABEL_NAME 30

#define IMMEDIATE_CODE 0
#define DIRECT_CODE 1
#define MATRIX_CODE 2
#define REGISTER_CODE 3
#define OPCODE_PADDING 6
#define SOURCE_PADDING 4
#define DEST_PADDING 2
#define IMMEDIATE_PADDING 2

#define ROW_REGISTER_PADDING_FOR_MATRIX 6
#define COLUMN_REGISTER_PADDING_FOR_MATRIX 2

#define REGISTER_SOURCE_PADDING 6
#define REGISTER_DEST_PADDING 2

#define CODE_MARK "code"
#define DATA_MARK "data"
#define EXTERNAL_MARK "external"

#define MAX_WORDS_PER_LINE 5

#include "preasm.h"


/*
 * This struct defines linked list represents all the labels defined in the program.
 * This is of course very useful (and crucial) for the second passage for the label address resolving
 */

typedef struct label_list{
    int value;
    char label_type[30];
    char label_name[LINE_LENGTH];
    struct label_list* next_label;
} label_list;

/*
 * This struct defines a linked list that contains each line of the code in it's bitwise representation.
 * The "value" is basically all the info you could think about a line:
 *      L = the amount of words it takes
 *      LC = the line number
 *      IC = it's part in the memory
 *      words = the literal bitwise representation of the line - has 5 words that default to zero
 *      labels + labels addressing - explained below. In a single line it's basically "where to put the resolution of
 *      label -> address"
 *      next - the next node
 *
 * */
typedef struct binary_line {
    int L;
    int LC;
    int IC;
    unsigned short words[MAX_WORDS_PER_LINE];

    /* this is a deviation from what is originally defined in the skeleton of the project
    the point of this is to mark which labels are in which line
    and where they should be inputted in the words list
    they struct will store them like this:
    labels[0] is for the source, if the source operand is a label
    labels[1] is for the destination
    both will are initialized to ''
    label addressing will save the space where the labels are supposed to be placed in words[]

    usage will be in the second passage like so:
    if labels[i] is label: i = {0, 1}
    one to one matching to labels_addressing
    words[labels_addressing[i]] = label address
    this will help us complete the final binary representation of the final word */

    char *labels[2];
    int labels_addressing[2];
    struct binary_line *next;
} binary_line;




void free_all(label_list *list, binary_line *line_to_free, macro_Linked_list *macro_table);
void print_binary(unsigned short number);
void first_passage(macro_Linked_list *macro_list, char *og_file_name, char *original_argv);

#endif /* FIRSTPASSAGE_H */
