//
// Created by Shauli on 06/06/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "secondpassage.h"
#include "firstpassage.h"

#include "preasm.h"
// 1, 7
#define IMMEDIATE_CODE 0
#define DIRECT_CODE 1
#define MATRIX_CODE 2
#define REGISTER_CODE 3
#define OPCODE_PADDING 6
#define SOURCE_PADDING 4
#define DEST_PADDING 2
#define IMMEDIATE_PADDING 2

#define ROW_REGISTER_PADDING_FOR_MATRIX 6
#define COLUMN_REGISTER_PADDING_FOR_MATRIX 2

#define REGISTER_SOURCE_PADDING 6
#define REGISTER_DEST_PADDING 2

int memory_pointer = 0;  // define memory pointer probably will change it later
int memory[256] = {0};

int is_null_terminated(const char *str, int max_len) {
    int i;
    if (str == NULL) {
        return 0;
    }
    for (i = 0; i < max_len; i++) {
        if (str[i] == '\0') {
            return 1;
        }
    }
    return 0;
}


/* apparently, strdup isn't a function in C90 lolll
 * so i built my own :)
 */
char *new_strdup(const char *str) {
    int length = (int)strlen(str) + 1; /* it cutted off the last part of my string probably null termination so add 1 */
    char *new_str = malloc(length);
    memcpy(new_str, str, length);
    return new_str;
}

void print_binary(unsigned short number) {
    int i;

    for (i = 9; i>=0; i--) {
        printf("%d", number & 1 << i ? 1 : 0);

        if (i == 6) {
            printf("-");
        }
        if (i == 4) {
            printf("-");
        }
        if (i == 2) {
            printf("-");
        }

    }
    printf(" ");

}

int number_of_psikim(char *line) {
    int i = 0;
    int sum = 0;
    for (i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') {
            sum++;
        }
    }
    return sum;
}


int name_valid(char *name, int LC, int *error) {
    int i = 0;
    const char *known_instructions[18] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne",
        "jsr", "red", "prn", "rts", "stop", "mcro", "mcroend"};

    if (strlen(name) > 30) {
        fprintf(stderr, "Error in line %d. The label name is longer than 30 chars.\n", LC);
        *error = 1;

        return 0;
    }
    if (isalpha(name[0]) == 0) {
        fprintf(stderr, "Error in line %d. The first char in name is a number.\n", LC);
        *error = 1;
        return 0;

    }
    for (i = 0; name[i] != '\0' && name[i] != '\n'; i++) {
        if (isalnum(name[i]) == 0 && name[i] != '_') {
            if (name[i] == ':' && name[i+1] != '\0') { // check if dots are in the name not in the last positon of name
                fprintf(stderr, "Error in line %d. The char in the %dth position in label name is not alphanumeric.\n"
                    , LC, i);
                *error = 1;
                return 0;
            }
            if (name[i] != ':') {
                fprintf(stderr, "Error in line %d. The char in the %dth position in label name is not alphanumeric.\n"
                    , LC, i);
                *error = 1;
                return 0;
            }
        }
    }
    // amud 28 lahzor
    if (name[strlen(name) - 1] == '\n') {
        name[strlen(name) - 1] = '\0'; // remove newline if there is any
    }
    for (i = 0; i < 18; i++) {
        if (is_null_terminated(name, LINE_LENGTH)) {
            if (strcmp(name, known_instructions[i]) == 0) { // if label name is known instruction name.
                fprintf(stderr, "Error in line %d. The label name is a known instruction name (%s).", LC,
                    known_instructions[i]);
                *error = 1;
                return 0;
            }
        }
        else {
            printf("hi! bad code! you should null terminate the string bro");
        }
    }
    return 1;
}

// remember to add more checks !!!
int is_symbol(char first_word[LINE_LENGTH], int *weird_flag, int LC, int *error) {
    int i = 0;
    char *possible_instructions[5] = {".data", ".string", ".mat", ".entry", ".extern"};
    if (first_word[strlen(first_word) - 1] == ':') {
        if (name_valid(first_word, LC, error)) {
            return 1;
        }
    }
    for (i = 0; i < 5; i++) { // check if there is an instruction .{} without label (rare)
        if (is_null_terminated(first_word, LINE_LENGTH)) {
            if (strcmp(first_word, possible_instructions[i]) == 0) {
                *weird_flag = 1;
                return 1;
            }
        }
        else {
            printf("hi! bad code! you have bad strings at %d", LC);
        }

    }
    return 0;

}

int is_instruction(char first_word[LINE_LENGTH], int LC, int *error) {
    const char *known_instructions[20] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend", ".entry", ".extern"};

    int i = 0;
    for (i = 0; i < 20; i++) {
        if (strcmp(first_word, known_instructions[i]) == 0) {
            return 1;
        }
    }
    fprintf(stderr, "Error: unknown instruction '%s' in line %d. \n", first_word, LC);
    *error = 1;
    return 0;

}

int is_data_storing(char first_word[LINE_LENGTH], char second_word[LINE_LENGTH]) {
    char *data_storers[3] = {".data", ".string", ".mat"};
    int i = 0;
    if (first_word[strlen(first_word) - 1] == ':') {
        int i = 0;
        for (i = 0; i < 3; i++) {
            if (strcmp(second_word, data_storers[i]) == 0) {
                return 1;
            }
        }
    }
    // check for weird case

    for (i = 0; i < 3; i++) {
        if (strcmp(first_word, data_storers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


void insert_to_label(label_list *list, char label_name[LINE_LENGTH], char *label_type, int *error, int DC) {
    // check if label name is in the list yet
    // checking each time for the next one - so i could add it at the end
    label_list *tmp = list;
    while (tmp->next_label != NULL) {
        if (strcmp(label_name, tmp->label_name) == 0) {
            *error = 1;
            fprintf(stderr, "Can't name two labels in the same name! The problem is with: '%s'\n", label_name);
        }
        tmp = tmp->next_label;
    }

    // means the final one does not exist
    // meaning "should" add name, doesn't throw error
    if (strcmp(label_name, tmp->label_name) != 0) {
        // insert into tmp the current label
        label_list *to_add = malloc(sizeof(label_list));
        int label_name_length = strlen(label_name);
        if (label_name[label_name_length-1] == ':') {
            label_name[label_name_length-1] = '\0';
        }
        strcpy(to_add->label_name, label_name);
        strcpy(to_add->label_type, label_type);
        to_add->value = DC;
        to_add->next_label = NULL;
        tmp->next_label = to_add;
        return;
    }
    fprintf(stderr, "couldn't insert into error");
    *error = 1;
}

void identify_data(char *data_type, char *data, int *DC, int *error, int LC) {
    char full_data[LINE_LENGTH] = "";
    int i = 0;
    int word_count = 0;
    int sign = 1;
    int num = 0;
    int appeared_space = 0;
    int appeared_numbers = 0;
    if (strcmp(data_type, ".string") != 0 && strcmp(data_type, ".data") != 0) {
        while (data != NULL) {
            strcat(full_data, data);
            data = strtok(NULL, " \t\n");
        }
    }
    else {
        while (data != NULL) {
            strcat(full_data, data);
            data = strtok(NULL, "");
        }
    }
    if (strcmp(data_type, ".data") == 0) { // if contaings numbers and such
        while (i < strlen(full_data)+1) {
            if ((full_data[i] == ',' && full_data[i + 1] == ',') || // shnei psikim beretzef yaani
                (full_data[i] == ',' && i == 0) || // psik the first
                (full_data[i] == ',' && i + 1 == strlen(full_data)) // psik the last :O
                ) {
                *error = 1;
                fprintf(stderr, "You have a problem with your commas :( it's on line %d. Fix it.\n", LC);
                return;
            }
            // new number and we know not two psikim next to each other
            if (full_data[i] == ',' || full_data[i] == '\0' || full_data[i] == '\n') {
                if (full_data[i] != '\0') {
                    memory[memory_pointer] = num * sign;
                    memory_pointer++;
                    word_count++; // new mila
                    sign = 1;
                    num = 0;
                    appeared_space = 0;
                    appeared_numbers = 0;
                }
            }
            // make sure the plus sign only appears next to a number or at the start of the expression :)
            // this is a weird expression but it just answers the requirement of this
            else if ((full_data[i] == '+' || full_data[i] == '-') &&
                ((full_data[i - 1] != ',' && full_data[i-1] != '\t' && full_data[i-1] != ' ')
                    || full_data[i + 1] == ' ' || full_data[i+1] == '\t') && i != 0) {
                *error = 1;
                fprintf(stderr, "You have a sign at a bad place :( at line %d\n", LC);
            }
            // check if all are numbers
            // + and - signs give false positive so account for these tooooooooooooooo
            else if (full_data[i] == ' ' || full_data[i] == '\t') {
                appeared_space = 1;
            }
            else if ((full_data[i] < '0' || full_data[i] > '9') && full_data[i] != '+' && full_data[i] != '-') {
                *error = 1;
                fprintf(stderr, "You have something that isn't a number in line %d", LC);
            }
            else if (full_data[i] == '-') {
                sign = -1;
            }
            else if (full_data[i] == '+') {
                sign = 1; // although it doesn't matter really
            }
            else {
                if (appeared_space && appeared_numbers) {
                    *error = 1;
                    fprintf(stderr, "You have a problem with your spaces :( it's on line %d. Fix it.\n", LC);
                    return;
                }
                appeared_numbers = 1;
                appeared_space = 0;
                num *= 10;
                num += full_data[i] - '0';
            }
        i++;
        }
    }
    if (strcmp(data_type, ".string") == 0) { // finished not ruining
        // count letters and account for quotes
        i = 0;
        // the condition after the || is for a very very rare case in which
        // the last line of the code is a string data storing decleration
        // without a name
        // for example .string "hello"
        while (full_data[i] != '\n' && full_data[i] != '\0') {
            if (i != 0 && (full_data[i+1] != '\n' || full_data[i+1] != '\0')) {
                // printf("added to memory: %d", (unsigned char)full_data[i]);
                memory[memory_pointer] = (unsigned char)full_data[i];
                memory_pointer++;
            }
            i++;
        }
        memory[memory_pointer - 1] = '\0';
        // now i is the length of the string so i-1 is supposed to be "
        if (full_data[0] == '"' && full_data[i-1] == '"') { // check if valid corners
            word_count += i - 1; // minus two quotes + '\0'
            // printf("I would've added %d words\n", word_count);
        }
    }
    if (strcmp(data_type, ".mat") == 0) {
        int first_number;
        int second_number;
        if (sscanf(full_data, "[%d][%d]", &first_number, &second_number) == 2) {
            word_count += first_number * second_number;
        }
        i = 6;
        while (i<strlen(full_data)+1) {
            if ((full_data[i] == ',' && full_data[i + 1] == ',') || // shnei psikim beretzef yaani
                (full_data[i] == ',' && i == 0) || // psik the first
                (full_data[i] == ',' && i + 1 == strlen(full_data)) // psik the last :O
                ) {
                *error = 1;
                fprintf(stderr, "You have a problem with your commas :( it's on line %d. Fix it.\n", LC);
                return;
                }
            // new number and we know not two psikim next to each other
            if (full_data[i] == ',' || full_data[i] == '\0' || full_data[i] == '\n') {
                memory[memory_pointer] = num * sign;
                memory_pointer++;
                word_count++; // new mila
                sign = 1;
                num = 0;
            }
            // make sure the plus sign only appears next to a number or at the start of the expression :)
            else if ((full_data[i] == '+' || full_data[i] == '-') && full_data[i - 1] != ',' && i != 0) {
                *error = 1;
                fprintf(stderr, "You have a sign at a bad place :( at line %d\n", LC);
            }
            // check if all are numbers
            // + and - signs give false positive so account for these tooooooooooooooo
            else if ((full_data[i] < '0' || full_data[i] > '9') && full_data[i] != '+' && full_data[i] != '-') {
                *error = 1;
                fprintf(stderr, "You have something that isn't a number in line %d", LC);
            }
            else if (full_data[i] == '-') {
                sign = -1;
            }
            else if (full_data[i] == '+') {
                sign = 1; // although it doesn't matter really
            }
            else {
                num *= 10;
                num += full_data[i] - '0';
            }
            i++;

        }
        // printf("I would've added %d words\n", word_count);
    }
    //update DC
    *DC += word_count;
}

int is_number(char *word) {
    int num_count = 0; // how many numbers are there
    int i = 1; // start from one because of #
    if (word[1] == '+' || word[1] == '-') { // if - or + ignore
        i = 2;
    }
    while (word[i] != '\0') {
        if (word[i] < '0' || word[i] > '9') {
            return 0;
        }
        i++;
        num_count++;
    }
    return num_count;
}

int is_register_range(char num) { // i can do one liner but nah not readable
    if (num < '0' || num > '7') {
        return 0;
    }
    return 1;
}

int analyze_matrix(char *op, char *first_bracket, char *second_bracket, int *error) {
    char matrix_label_name[LINE_LENGTH];
    char junk[LINE_LENGTH];
    // want to catch if there are stuff outside the matrix we don't want ! for example letters after the matrix and so on
    int catch = sscanf(op, "%[^[][%[^]]][%[^]]]%s", matrix_label_name, first_bracket, second_bracket, junk);
    if (catch == 3) {
        strcpy(op, matrix_label_name);
        return 1;
    }
    if (catch > 3) {
        printf("caught");
    }
    return 0;
}

// explanation:
// i'm going to firs analyze the first operand and check what it is
// then i'm goin to analyze the second operand
// and then i'm going to do some ifs to check ifs to check which case is happening and
// and add to L accordingly.

int words_per_operand(char *operand, int *operand_type, int *error, int *register_flag, int LC) {
    // check if immediate
    // using fillers for the anlyze matrix function
    char filler[LINE_LENGTH];
    char matrix_tmp[LINE_LENGTH];
    if (operand[0] == '#') {
        if (is_number(operand)) {
            //printf("line number %d added 1 word\n", LC);
            *operand_type = IMMEDIATE_CODE;
            return 1;
        }
        *error = 1;
        fprintf(stderr, "You have something that isn't a number in line %d", LC);
        return 0;
    }
    // check if register
    if (operand[0] == 'r' && is_register_range(operand[1])) {
        if (*register_flag == 0) {
            *register_flag = 1;
            *operand_type = REGISTER_CODE;
            //printf("line number %d added 1 word\n", LC);
            return 1;
        }
        *operand_type = REGISTER_CODE;
        return 0;
    }

    strcpy(matrix_tmp, operand);
    if (analyze_matrix(matrix_tmp, filler, filler, error)) { // idk if this is common c practice
        *operand_type = MATRIX_CODE;
        return 2;
    }
    // in this case, the first operand is not a matrix, direct, or register.
    // there are two possible scenarios - it's a label or it's junk
    // for now, let's assume it's a label and in the second maavar we will check if it's ok or not
    if (name_valid(operand, LC, error)){
        *operand_type = DIRECT_CODE;
        return 1; // in the case of a label
    }
    *error = 1;
    fprintf(stderr, "bad operand");
    return 0;
}

int analyze_operands(char *instruction, char *first_op, char *second_op, int *source_mion, int *dest_mion,
    int *error, int LC) {
    int L = 1;
    int found_register = 0; // because we want to make sure we only add one word for registers (in case there are two)

    // if command with one op
    if (!second_op) {
        source_mion = dest_mion;
    }
    // check first operand:
    if (first_op) {
        L += words_per_operand(first_op, source_mion, error, &found_register, LC);
        //printf("first operand is %s and took %d ", first_op, L);
    }
    if (second_op) {
        L += words_per_operand(second_op, dest_mion, error, &found_register, LC);
        //printf("second operand is %s and took %d ", second_op, L);
    }
    // printf("total: %d\n", L);

    return L;

}


unsigned short make_binary_line(char *instruction, int *first_op_type, int *second_op_type) {
    // first analyze instruction
    // then analyze the immidiate mioon
    // how i do it:
    // first, get A,R,E
    // second get mioon of dest operand
    // third get mioon of source operand
    // lastly get the opcode
    // in the first passage all i need to do it just set the source if its immediate and the opcode
    // ARE will always be 00 for the first passage !
    // so i won't touch it

    unsigned short dest_type;
    unsigned short source_type;
    unsigned short opcode;
    unsigned short final_bit;
    int i = 0;

    // OPCODE
    // the idea with this is to make the index of each instruction the number and then transform i to binary
    const char *known_instructions[16] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne",
        "red", "prn", "jsr", "rts", "stop"};
    for (i = 0; i<16; i++) {
        if (strcmp(instruction, known_instructions[i]) == 0) {
            break;
            // printf("the instruction is: %s and its hex opcode is %d\n", instruction, i);
        }
    }
    // work on shifting logic

    dest_type = *second_op_type << DEST_PADDING;
    source_type = *first_op_type << SOURCE_PADDING;
    opcode = i << OPCODE_PADDING;
    final_bit = opcode | dest_type | source_type;
    // printf("finbal bitwise is: %d \n", final_bit);
    // 1100 00 00 00
    // 1001 00 01 00

    // handle first word in case of direct addressing
    // in theory i would've liked to check if source operand is 00
    // but i also need to check the other one is not 00 too
    // because commands like stop and others have 00 in the source to
    // print_binary(final_bit);

    return final_bit;
}

unsigned short make_word(char *operand) {
    unsigned short sum = 0;
    int is_negative = 1;
    int i = 1; // start from the second number
    if (operand[1] == '-') {
        is_negative = -1;
        i++;
    }
    if (operand[1] == '+') {
        i++;
    }
    while (operand[i] != '\0') {
        sum *= 10;
        sum += operand[i] - '0';
        i++;
    }
    sum *= is_negative;
    // make it fit 10 bits for the word
    if (is_negative == -1) {
        sum = sum << IMMEDIATE_PADDING;
        // & it with 0's in the 15-10th position (in case of negative)
        // so i need 0000001111111111 to zero-out all the 15-10 bits
        // in hex it's 3FF
        sum = sum & 0x3FF;
    }
    else {
        sum = sum << IMMEDIATE_PADDING;
    }

    return sum;
}

unsigned short make_matrix_word(char *first_bracket, char *second_bracket) {
    // 6-9 = shoora
    // 5-2 = amooda
    int row = first_bracket[1] - '0';
    int column = second_bracket[1] - '0';
    unsigned short final_bit;
    // format it to 6-9, 5-2
    row = row << ROW_REGISTER_PADDING_FOR_MATRIX;
    column = column << COLUMN_REGISTER_PADDING_FOR_MATRIX;

    final_bit = row | column;
    return final_bit;
}

// checks if a string is in a list
//

int contains_in_list(const char *str1, const char **list, int const size) {
    int i;
    for (i = 0; i<size; i++) {
        if (strcmp(str1, list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// this function checks that the instruction has enough operands
// for example - the instructino mov requires two operands
// this function will hopefully catch all the cases where an instruction requires n operands and make sure
// the call for the instruction satisfies the operand requirements
// returns 1 if call is invalid and 0 if valid
int invalid_call(char *instruction, char *first_op, char *second_op) {
    const char *two_operands[5] = {"mov", "cmp", "add", "sub", "lea"};
    const char *single_operand[9] = { "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn"};
    const char *no_operands[2] = {"rts", "stop"};

    // check two operands first
    if (contains_in_list(instruction, two_operands, 5)) { // this means the instruction should have two operands
        if (first_op != NULL && second_op != NULL) {
            return 0; // call is valid
        }
        return 1; // call is invalid
    }
    if (contains_in_list(instruction, single_operand, 9)) {
        if (first_op != NULL && second_op == NULL) { // only first operand exists
            return 0; // call is valid
        }
        return 1; // call is invalid
    }
    if (contains_in_list(instruction, no_operands, 2)) {
        if (first_op == NULL && second_op == NULL) { // there shouldn't be any operands
            return 0; // call is valid
        }
        return 1;
    }

    return 0;
}

void analyze_and_build(int *L, unsigned short *line_binary_representation, unsigned short *immediate_word,
    char *instruction, char *first_op, char *second_op, int *source_mion, int *dest_mion, int *error, int LC,
    binary_line **binary_list, int IC) {
    char first_bracket[LINE_LENGTH];
    char second_bracket[LINE_LENGTH];
    int source_r_number = 0;
    int dest_r_number = 0;
    int i;
    unsigned short matrix_word;
    binary_line* new_node = malloc(sizeof(binary_line));

    *L = analyze_operands(instruction, first_op, second_op, source_mion,
                            dest_mion, error, LC); // 13 dest and source nonsense fix it in the original function
    *line_binary_representation = make_binary_line(instruction, source_mion,
        dest_mion); // 14
    if (invalid_call(instruction, first_op, second_op)) { // this means the call is invalid
        *error = 1;
        fprintf(stderr, "Error. The number of operands for the command in line %d is bad", LC);
    }
    // 15

    new_node->LC = LC;
    new_node->L = *L;
    new_node->IC = IC;
    new_node->words[0] = *line_binary_representation;
    new_node->next = NULL;

    for (i = 1; i < 5; i++) {
        new_node->words[i] = 0;
    }
    // initialzie the type for the labels
    for (i = 0; i < 2; i++) {
        new_node->labels[i] = "";
        new_node->labels_addressing[i] = 0;
    }
    // if immidiate
    if (first_op != NULL && first_op[0] == '#') {
        // printf("this is immidiate\n");
        *immediate_word = make_word(first_op);
        new_node->words[1] = *immediate_word;
    }

    if (*source_mion == 2) {
        analyze_matrix(first_op, first_bracket, second_bracket, error);
        new_node->words[2] = make_matrix_word(first_bracket, second_bracket);
        new_node->labels[0] = new_strdup(first_op);
        new_node->labels_addressing[0] = 1; // words[1] needs to be updated to the location of the matrix
        //printf("sixth added %s intended to be placed in word place %d\n", new_node->labels[0], new_node->labels_addressing[0]);
    }
    if (*dest_mion == 2) {
        analyze_matrix(second_op, first_bracket, second_bracket, error);
        matrix_word = make_matrix_word(first_bracket, second_bracket);
        if (*source_mion == 2) {
            // words[0] is instruction, words[1-2] is the source matrix , words[3] is matrix and 4 is word
            new_node->words[4] = matrix_word; // this means two matrixes
            new_node->labels[1] = new_strdup(second_op);
            new_node->labels_addressing[1] = 3; // words[3] needs to be updated to the location of the matrix
            //printf("fifth added %s intended to be placed in word place %d\n", new_node->labels[1], new_node->labels_addressing[1]);
        }
        else { // if register or immediate
            // words[0] is instruction, words[1] is the source (register or immediate), words[2] is matrix and 3 is word
            new_node->words[3] = matrix_word;
            new_node->labels[1] = new_strdup(first_op);
            new_node->labels_addressing[1] = 2; // words[2] needs to be updated to the location of the matrix
            //printf("forth added %s intended to be placed in word place %d\n", new_node->labels[1], new_node->labels_addressing[1]);
        }
    }
    if (first_op != NULL && *source_mion == 3) {
        source_r_number = first_op[1] - '0';
        // format it to 6-9
        source_r_number = source_r_number << REGISTER_SOURCE_PADDING;
        new_node->words[1] = source_r_number; //
    }
    if (second_op != NULL && *dest_mion == 3) {
        dest_r_number = second_op[1] - '0';
        // format it to 6-9
        dest_r_number = dest_r_number << REGISTER_DEST_PADDING;
        if (*source_mion == 0 || *source_mion == 1) {
            new_node->words[2] = dest_r_number; // words[0] is instruction words[1] is immediate
        }
        else if (*source_mion == 2) { // if matrix
            new_node->words[3] = dest_r_number; // 0 is instruction, 1 is matrix, 2 is matrix registers 3 is destiantion
        }
        else if (*source_mion == 3) {
            new_node->words[1] = new_node->words[1] | dest_r_number; // same extra word
        }
    }
    if (*source_mion == 1) { // if label is source
        // now in the first passage words[1] is by default 0
        // so we update labels and labels addressing accordingly:
        new_node->labels[0] = new_strdup(first_op); // first operand is a label, "source"
        new_node->labels_addressing[0] = 1; // 0 is source and words[1] needs to be read as label
        //printf("first added %s intended to be placed in word place %d\n", new_node->labels[0], new_node->labels_addressing[0]);
    }
    if (*dest_mion == 1) {
        if (*source_mion == 2) { // if matrix is source and destination is label
            // because if for example jmp or any of the one operand instruciton so the first op is the destination
            if (second_op == NULL) {
                new_node->labels[1] = new_strdup(first_op);
            }
            else {
                new_node->labels[1] = new_strdup(second_op);
            }
            new_node->labels_addressing[1] = 3; // 3 because 0 - instruction; 1 - matrix name; 2 - matrix registers
            //printf("second added %s intended to be placed in word place %d\n", new_node->labels[1], new_node->labels_addressing[1]);

        }
        else {
            // because if for example jmp or any of the one operand instruciton so the first op is the destination
            if (second_op == NULL) {
                new_node->labels[1] = new_strdup(first_op);
                new_node->labels_addressing[1] = 1; // 0 - instruction; 1 - destination
            }
            else {
                new_node->labels[1] = new_strdup(second_op);
                new_node->labels_addressing[1] = 2; // 0 - instruction; 1 - source operand 2 - our label
            }
            //printf("third added %s intended to be placed in word place %d\n", new_node->labels[1], new_node->labels_addressing[1]);
        }
    }

    if ((*binary_list)->LC == -1) { // if dummy node
        **binary_list = *new_node; // workaround
        free(new_node);
    }
    else {
        (*binary_list)->next = new_node;
        *binary_list = new_node;
    }

}

void free_all(label_list *list, binary_line *line_to_free) {
    while (line_to_free != NULL) {
        binary_line *tmp = line_to_free->next;
        int i;
        printf("\n");

        for (i = 0; i<5; i++) {
            print_binary(line_to_free->words[i]);
        }

        if (strcmp(line_to_free->labels[0], "") != 0) {
            free(line_to_free->labels[0]);
        }
        if (strcmp(line_to_free->labels[1], "") != 0) {
            free(line_to_free->labels[1]);
        }
        free(line_to_free);
        line_to_free = tmp;
    }
    free(line_to_free);
    printf("\n");
    while (list->next_label != NULL) {
        label_list *tmp = list->next_label;
        printf("%s | %d | %s\n", tmp->label_name, tmp->value, tmp->label_type);
        free(list);
        list = tmp;
    }
    free(list);


}

int main() {
    int DC = 0;
    int IC = 100;
    int LC = 1; // line counter haha
    int exists_error = 0;
    int exists_label = 0;
    int w;
    char *first_word;
    char *second_word;
    char *third_word = 0;
    int weird_symbol_flag = 0; // in case there is a "weird" symbol (data wasting space for example .data)
    int DCF;
    int ICF;
    label_list *tmp;
    binary_line *instructions_iter;
    char line[LINE_LENGTH];

    FILE *source_asm = fopen("postpre.asm", "r");
    label_list* the_label_list = malloc(sizeof(label_list));
    binary_line* instructions_in_binary = malloc(sizeof(binary_line));
    // define empty head
    the_label_list->next_label = NULL;
    strcpy(the_label_list->label_name,"");
    strcpy(the_label_list->label_type,"");

    instructions_in_binary->LC = -1; // mark dummy node
    instructions_in_binary->L = 0;
    instructions_in_binary->IC = IC;
    instructions_in_binary->next = NULL;
    for (w = 0; w < 5; w++) {
        instructions_in_binary->words[w] = 0;
    }
    for (w = 0; w < 2; w++) {
        instructions_in_binary->labels[w] = "";
        instructions_in_binary->labels_addressing[w] = 0;
    }

    instructions_iter = instructions_in_binary;



    while (fgets(line, LINE_LENGTH, source_asm) && !exists_error) {
        char line_for_tokenisation[LINE_LENGTH]; // line to not ruin the original string
        strncpy(line_for_tokenisation, line,LINE_LENGTH);
        first_word = strtok(line_for_tokenisation, " \t\n");
        if (first_word == NULL) { // empty line
            LC++;
            continue;
        }
        if (first_word[0] == ';') { // comment
            LC++;
            continue;
        }

        // if we get string as data we want it not to erase the spaces
        // account for this early on

        if (strcmp(first_word, ".string") == 0 || strcmp(first_word, ".data") == 0) { // this is only for the weird symbol case.
            second_word = strtok(NULL, "");
        }
        else { // jert
            second_word = strtok(NULL, " ,\t\n");
        }
        third_word = 0;
        if (second_word) {
            if (strcmp(second_word, ".string") == 0 || strcmp(second_word, ".data") == 0) {
                third_word = strtok(NULL, "");
            }
            else {
                third_word = strtok(NULL, " \t\n");
            }
        }

        // this is for the case of wasting space in assembly and defining a line like this:
        // .data "somedata"
        // for some reason it's possible, in the project handbook i didn't see any refrence to it
        // but i guess i should take care of it too no?
        weird_symbol_flag = 0; // in case there is a "weird" symbol (data wasting space for example .data)

        exists_label = is_symbol(first_word, &weird_symbol_flag, LC, &exists_error);
        if (is_data_storing(first_word, second_word)) { // 5
            if (exists_label) {
                // in the case that the decleration is only .data, we will call without the name (first_word)
                insert_to_label(the_label_list, first_word, "data", &exists_error, DC); // 6
                if (weird_symbol_flag) {
                    identify_data(first_word, second_word, &DC, &exists_error, LC);
                }
                else {
                    identify_data(second_word, third_word, &DC, &exists_error, LC); // 7
                }
            }
        }
        // non-data (entry and extern) are always weird flag cases
        else if (weird_symbol_flag) { // if not data (line 8 // actually it is always here no need
            if (strcmp(first_word, ".entry") == 0) { // 9
                LC++;
                continue;
            }
            // if i was arrogant i would've left this if out of the code because if it reached here
            // and it is a symbol and clearly is not any of the other 4 options so it's for sure
            // insert_to_label(label_list *list, char label_name[LINE_LENGTH], int *error, int DC)
            if (strcmp(first_word, ".extern") == 0) {
                // 10
                insert_to_label(the_label_list, second_word, "external", &exists_error, 0);
            }

        }
        else { // instruction without label ! // this means that first word has to be a known instruction
            unsigned short line_binary_representation;
            unsigned short immediate_word;
            int dest_mion = 0;
            int source_mion = 0;
            int L = 0;

            if (exists_label) {
                insert_to_label(the_label_list, first_word, "code", &exists_error, IC); // 11
                if (is_instruction(second_word, LC, &exists_error)) { // 12
                    // check for stop or rts
                    char *forth_word = strtok(NULL, " ,\t\n");
                    if ((strcmp(second_word, "stop") == 0 || strcmp(second_word, "rts") == 0) && third_word == NULL) {
                        analyze_and_build(&L, &line_binary_representation, &immediate_word, second_word, third_word,
                            forth_word, &source_mion, &dest_mion, &exists_error, LC, &instructions_iter, IC);
                    }
                    // 13
                    // a problem has arisen that i didn't take into consideration:
                    // sometimes there will be a "," without a space
                    // i don't know if it's allowed or not but let's assume it is
                    // i'll strtok it again
                    else {
                        if (forth_word == NULL) {
                            char copy_third[LINE_LENGTH]; // in case no space between the psikim
                            strncpy(copy_third, third_word,LINE_LENGTH);
                            third_word = strtok(copy_third, " ,");
                            forth_word = strtok(NULL, " ,");
                            // printf("splitted here %s", forth_word);
                        }
                        analyze_and_build(&L, &line_binary_representation,  &immediate_word, second_word, third_word,
                            forth_word, &source_mion, &dest_mion, &exists_error, LC, &instructions_iter, IC);

                    }
                    // error checking
                    if (number_of_psikim(line) == 0 && forth_word != NULL && third_word != NULL) { //
                        fprintf(stderr, "Error in line %d. No psikim between operands.\n", LC);
                        exists_error = 1;
                    }
                }


            }
            else if (is_instruction(first_word, LC, &exists_error)) { // 12
                if ((strcmp(first_word, "stop") == 0 || strcmp(first_word, "rts") == 0) && second_word == NULL) {
                    analyze_and_build(&L, &line_binary_representation, &immediate_word, first_word, second_word,
                        third_word, &source_mion, &dest_mion, &exists_error, LC, &instructions_iter, IC);
                }
                else {
                    if (third_word == NULL) {
                        char copy_second[LINE_LENGTH]; // in case no space between the psikim
                        strncpy(copy_second, second_word,LINE_LENGTH);
                        second_word = strtok(copy_second, " ,");
                        third_word = strtok(NULL, " ,");
                    }
                    analyze_and_build(&L, &line_binary_representation, &immediate_word, first_word, second_word,
                                            third_word, &source_mion, &dest_mion, &exists_error, LC, &instructions_iter, IC);
                }
                // if we have both a destination (third) and source (second) words and no psik in the line
                // means error
                // if a psik is somewhere else in the code, it will raise label not in list error

                if (number_of_psikim(line) == 0 && third_word != NULL && second_word != NULL) {
                    fprintf(stderr, "Error in line %d. No psikim between operands.\n", LC);
                    exists_error = 1;
                }
            }
            // else {
            //
            // }
            IC += L;
            // printf("the line: %s took %d words making the final word count %d", line, L, DC);
        }

    LC++;
    }

    if (exists_error) { // 17
        free_all(the_label_list, instructions_in_binary);
        fclose(source_asm);
        exit(1);
    }
    fclose(source_asm);

    DCF = DC;
    ICF = IC;

    tmp = the_label_list;

    while (tmp != NULL) {
        if (strcmp(tmp->label_type, "data") == 0) {
            tmp->value = tmp->value + ICF;
            // printf("hi! ium %s and take %d\n",tmp->label_name, tmp->value);
        }
        tmp = tmp->next_label;
    }
    second_passage(instructions_in_binary, the_label_list, DCF, ICF);
    return 0;
}
