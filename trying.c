//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "preasm.h"
#include <math.h>



int main() {

    int second_op_type = 1;
    int first_op_type = 1;
    int i = 0;

    unsigned short dest_type = second_op_type << 2;
    unsigned short source_type = first_op_type << 4;
    unsigned short opcode = i << 6;
    unsigned short final_bit = opcode | dest_type | source_type;
    printf("%d", final_bit);

}
