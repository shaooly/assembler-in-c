//
// Created by Shauli on 07/06/2025.
//

#ifndef PREASM_H
#define PREASM_H

#define LINE_LENGTH 81


typedef struct Linked_List {
    char instruction[LINE_LENGTH];
    struct Linked_List* next_instruction;
} Linked_List;

typedef struct macro_Linked_list {
    char name[30];
    Linked_List* first_instruction;
    struct macro_Linked_list* next_macro;
} macro_Linked_list;

extern macro_Linked_list global_macro;


#endif //PREASM_H
