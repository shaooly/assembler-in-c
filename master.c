/*
 * Master file to run the assembler.
 * The code scans the input of the user when running the assembler.
 * Usage is: {the name of the exe} {file1} {file2} ...
 * Notice: if you want to assemble file1.as you should run it with "assembler file"
 *
 * Author: Shaul Joseph Sasson
 * Created: 15/08/2025
 *
 * Depends on master.h, preasm.h, firstpassage.h, memory.h
 *
 */

#include "master.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preasm.h"
#include "firstpassage.h"
#include "memory.h"



void assemble_files(int argc, char *argv[]) {
    int j;
    size_t len;
    macro_Linked_list *macro_table;
    char *file_name;
    char *post_file_name;
    FILE *tmp;
    if (argc < 2) {
        fprintf(stderr, "Error: you should use it like this: %s <file> [<file> ...]\n", argv[0]);
        return;
    }
    for (j = 1; j<argc; j++) {
        clean_memory();
        len = strlen(argv[j]) + 3 + 1; /* 3 for .as and + 1 for \0 */
        file_name = malloc(len);
        if (!file_name) {
            fprintf(stderr, "Error: not enough memory\n");
            continue;
        }

        /* this is responsible for opening the original file*/
        strcpy(file_name, argv[j]);
        strcat(file_name, ".as");

        /* make sure the file exists */
        tmp = fopen(file_name, "r");
        if (!tmp) {
            fprintf(stderr, "Error: cannot open file %s\n", file_name);
            free(file_name);
            continue;
        }
        fclose(tmp);

        /* this makes the file post the macros spread */
        post_file_name = malloc(len + 4 + 1); /* 4 for the word post + 1 for the \0 */
        if (!post_file_name) {
            fprintf(stderr, "Error: not enough memory\n");
            free(file_name);
            continue;
        }
        strcpy(post_file_name, "post");
        strcat(post_file_name, file_name);

        /* post_file_name = post{ogfilename}.as */
        macro_table = pre_asm(file_name, post_file_name);
        if (!macro_table) {
            free(post_file_name);
            free(file_name);
            continue;
        }
        /* pass the original argv because i don't know how to strip the "post" and ".as"*/
        first_passage(macro_table, post_file_name, argv[j]);
        free(post_file_name);
        free(file_name);
    }
}

int main(int argc, char *argv[]) {
    assemble_files(argc, argv);
    return 0;
}
