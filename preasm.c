//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *get_first_word(char *line) {
    int i = 0;
    for (i = 0; line[i] == ' ' || (int)line[i] == '\t'; i++) {} // advance i to the first non tab or space char
    char *first_word = strtok(&line[i], " ");
    char *second_word = strtok(NULL, "\t");
    printf("the first word is %s and the second word is %s\n", first_word, second_word);

    return "";
}


int main(void) {
    char *filename = "source.asm";
    int IC = 1;
    FILE *source_asm = fopen(filename, "r");
    FILE *post_pre_asm = fopen("postpre.asm", "w");
    char line[81]; // assuming max line length and is 81 characters long including "\n"


    while (fgets(line, 81, source_asm)) {
        // check if line is longer than 81 chars
        if (strstr(line, "\n") == NULL) { // reached end of fgets count amount of chars without end line.
            if (fgetc(source_asm) == EOF) { // when last line doesn't have "\n"
                exit(9);
            }
            fprintf(stderr, "Error in line %d. The instruction is longer than 80 chars.\n", IC);
            exit(1);
        }

        get_first_word(line);

        IC++;
    }
    fclose(source_asm);
    fclose(post_pre_asm);
    exit(9);
}