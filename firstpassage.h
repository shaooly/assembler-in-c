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
    int IC;
    unsigned short words[5];
    struct binary_line *next;
} binary_line;

#endif //FIRSTPASSAGE_H
