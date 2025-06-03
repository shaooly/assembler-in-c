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

int contains(macro_Linked_list* macro_table, char* command_name) {
    macro_Linked_list* iteratepoint = macro_table;
    while (iteratepoint != NULL) {
        if (strcmp(command_name, iteratepoint->name) == 0) {
            return 1;
        }
        macro_Linked_list* trmp = iteratepoint->next_macro;
        iteratepoint = trmp;
    }
    return 0;
}


int main() {
    macro_Linked_list* macro_table = malloc(sizeof(macro_Linked_list));
    // pointer to the begging of the table to "save" the original pointer;
    macro_Linked_list* mcropoint = macro_table;

    strcpy(mcropoint->name, "shaooly");
    mcropoint->first_instruction = NULL;
    mcropoint->next_macro = NULL;

    macro_Linked_list *to_add = malloc(sizeof (macro_Linked_list));
    to_add->next_macro = NULL;
    to_add->first_instruction = NULL;
    strcpy(to_add->name, "freinbach");
    mcropoint->next_macro = to_add;
    free(to_add);
    free(macro_table);
    printf("%d", contains(mcropoint, ""));
}
