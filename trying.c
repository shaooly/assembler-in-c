//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char *nigg = "hsss";
    char nigga[82];
    strcpy(nigga, nigg);
    printf("%s\n", nigga);
    char mystring[] = "         Hello                World";
    char *tok = strtok(mystring, " ");
    printf("%s", tok);
    while (tok != 0) {
        printf("%s\n", tok);

        // Use of strtok
        // go through other tokens
        tok = strtok(0, " ");
    }

    return 0;
}