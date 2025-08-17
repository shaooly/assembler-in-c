/*
 * Master file to manage the memory.
 * There isn't a lot going on here but it's cleaner then to write it in the master file.
 *
 * Basically creates the memory and is responsible for cleaning it too.
 *
 * Author: Shaul Joseph Sasson
 * Created: 15/08/2025
 * Depends on memory.h
 *
 */
#include "memory.h"

int memory_pointer = 0;  /*  define memory pointer probably will change it later */
int memory[MEMORY_SIZE] = {0};

void clean_memory() {
    int w;
    for (w = 0; w < MEMORY_SIZE; w++) {
        memory[w] = 0;
    }
    memory_pointer = 0;
}
