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
    unsigned short words[5];

    // this is a deviation from what is originally defined in the skeleton of the project
    // the point of this is to mark which labels are in which line
    // and where they should be inputted in the words list
    // they struct will store them like this:
    // labels[0] =
    char *labels[2];
    char *labels_addressing[2];
    struct binary_line *next;
} binary_line;

extern unsigned short memory[256];
extern int memory_pointer;

#endif //FIRSTPASSAGE_H
