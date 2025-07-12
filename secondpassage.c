//
// Created by Shauli on 11/07/2025.
//

#include "secondpassage.h"
#include "firstpassage.h"

int main() {
    int exists_error = 0;
    FILE *source_asm = fopen("postpre.asm", "r");

    char line[LINE_LENGTH];

    while (fgets(line, LINE_LENGTH, source_asm) && !exists_error) {
        printf("%s", line);
        if (line[0] == '\n') {
            printf("hi");
        }
    }
    return 0;
}


