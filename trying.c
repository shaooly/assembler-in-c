//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "preasm.h"



int main() {
    unsigned short dest_type = 1 << 3;
    unsigned short source_type = 1 << 5;
    unsigned short opcode = 10 << 6;
    unsigned short final_bit = opcode | dest_type | source_type;
    printf("%d", final_bit);

}
