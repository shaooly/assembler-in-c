//
// Created by Shauli on 11/07/2025.
//

#define LINE_LENGTH 81

#ifndef FIRSTPASSAGE_H
#define FIRSTPASSAGE_H

typedef struct label_list{
    int value;
    char label_type[30];
    char label_name[LINE_LENGTH];
    struct label_list* next_label;
} label_list;

typedef struct binary_line {
    int L;
    int LC;
    int IC;
    unsigned short words[5];

    // this is a deviation from what is originally defined in the skeleton of the project
    // the point of this is to mark which labels are in which line
    // and where they should be inputted in the words list
    // they struct will store them like this:
    // labels[0] is for the source, if the source operand is a label
    // labels[1] is for the destination
    // both will are initialized to ''
    // label addressing will save the space where the labels are supposed to be placed in words[]

    // usage will be in the second passage like so:
    // if labels[i] is label: i = {0, 1}
    // one to one matching to labels_addressing
    // words[labels_addressing[i]] = label address
    // this will help us complete the final binary representation of the final word
    // i hope this is detailed enough

    char *labels[2];
    int labels_addressing[2];
    struct binary_line *next;
} binary_line;




void free_all(label_list *list, binary_line *line_to_free);
void print_binary(unsigned short number);

extern int memory[256];
extern int memory_pointer;

#endif //FIRSTPASSAGE_H
