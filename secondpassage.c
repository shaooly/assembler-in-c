//
// Created by Shauli on 11/07/2025.
//

#include "secondpassage.h"
#include "firstpassage.h"

int memory_pointer;


int main() {
    int exists_error = 0;
    int LC = 0; // we will match it with LC's !
    FILE *source_asm = fopen("postpre.asm", "r");
    printf("memory: %d", memory_pointer);

    char line[LINE_LENGTH];

    while (fgets(line, LINE_LENGTH, source_asm) && !exists_error) {
        printf("%s", line);
        if (line[0] == '\n') {
            printf("hi");
        }
    }
    return 0;
}


