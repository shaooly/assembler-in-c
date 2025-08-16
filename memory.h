/*
 * Header file for the memory management.
 * Author: Shaul Joseph Sasson
 */

#ifndef MEMORY_H
#define MEMORY_H
#define MEMORY_SIZE 256

extern int memory[MEMORY_SIZE];
extern int memory_pointer;

void clean_memory();

#endif //MEMORY_H
