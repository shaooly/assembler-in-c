//
// Created by Shauli on 30/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "preasm.h"
#include <math.h>

// void analyze_and_build(int *L, unsigned short *line_binary_representation, char *instruction, char *first_op,
//     char *second_op, int *source_mion, int *dest_mion, int *error, int IC) {
//     *L = analyze_operands(instruction, first_op, second_op, dest_mion,
//                             source_mion, error, IC); // 13 dest and source nonsense fix it in the original function
//     *line_binary_representation = make_binary_line(instruction, source_mion,
//         dest_mion); // 14
// }
//
//
// int main() {
//
//     int second_op_type = 1;
//     int first_op_type = 1;
//     int i = 0;
//
//     unsigned short dest_type = second_op_type << 2;
//     unsigned short source_type = first_op_type << 4;
//     unsigned short opcode = i << 6;
//     unsigned short final_bit = opcode | dest_type | source_type;
//     printf("%d", final_bit);
// 9
// }
//