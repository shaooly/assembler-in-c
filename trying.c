//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

typedef struct Linked_List{
    char instruction[81];
    struct Linked_List* next_instruction;
} Linked_List;

typedef struct  macro_Linked_list{
    char name[30];
    struct Linked_List* first_instruction;
    struct macro_Linked_list* next_macro;
}macro_Linked_list;


int main() {
    macro_Linked_list macro_table;
    // pointer to the begging of the table to "save" the original pointer;
    macro_Linked_list* mcropoint = &macro_table;
    Linked_List first_ins;

    // this is the scnd instruction
    strcpy(first_ins.instruction, "mov r1, r2");

    // this is the first instruction
    Linked_List second_ins;
    strcpy(second_ins.instruction, "inc r1");
    first_ins.next_instruction = &second_ins;

    // third
    Linked_List third;
    strcpy(third.instruction, "M1:		.mat [2][2] 1,2,3,4");
    second_ins.next_instruction = &third;
    third.next_instruction = NULL;


    // create mcropoint
    strcpy(mcropoint->name, "shaooly");
    mcropoint->first_instruction = &first_ins;
    mcropoint->next_macro = NULL;

    // printf("%s\n", mcropoint->first_instruction->instruction);
    // printf("%s\n", mcropoint->first_instruction->next_instruction->instruction);
    // printf("%s\n", mcropoint->first_instruction->next_instruction->next_instruction->instruction);
    /// soff soff srabak ars



    macro_Linked_list* iteratepoint = &macro_table;
    // first print out of loop
    // printf("%s", iteratepoint->first_instruction->instruction);

    Linked_List* iterate_linked = iteratepoint->first_instruction;

    while (iterate_linked!=NULL) {
        printf("%s\n", iterate_linked->instruction);
        iterate_linked = iterate_linked->next_instruction;
    }
}
