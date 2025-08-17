/*
 * Header file for the pre-assembler.
 * Author: Shaul Joseph Sasson
 */

#ifndef PREASM_H
#define PREASM_H

#define LINE_LENGTH 81 /* 80 + '\0'*/
#define MAX_MACRO_LEN 30
#define INSTRUCTION_AMOUNT 18

/* linked list for the macros
 * each macro will have this linked list as the "value"
 * basically each linked list in the macro list represents an entire macro
 */

typedef struct Linked_List {
    char instruction[LINE_LENGTH];
    struct Linked_List* next_instruction;
} Linked_List;

/* macro linked list contains the name of the macro and a linked list
 * as described earlier to contain the instructions
 */

typedef struct macro_Linked_list {
    char name[MAX_MACRO_LEN];
    Linked_List* first_instruction;
    struct macro_Linked_list* next_macro;
} macro_Linked_list;

macro_Linked_list* pre_asm(char *filename, char *post_filename);


#endif /* PREASM_H */
