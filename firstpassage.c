//
// Created by Shauli on 06/06/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "secondpassage.h"
#include "firstpassage.h"
#include "memory.h"


#include "preasm.h"
// 1, 7


/* apparently, strdup isn't a function in ansi C
 * so i built my own :)
 */
char *new_strdup(const char *str) {
    int length = (int)strlen(str) + 1; /* it cutted off the last part of my string probably null termination so add 1 */
    char *new_str = malloc(length);
    memcpy(new_str, str, length);
    return new_str;
}

// helper function
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

// counts the number of psikim in a string

int number_of_psikim(const char *line) {
    int i = 0;
    int sum = 0;
    for (i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') {
            sum++;
        }
    }
    return sum;
}

/* Validate a label name; checks length/syntax and conflicts with macros, opcodes, and registers.
* Params:
 *      name - the suspected label
 *      LC - the current line
 *      macro_table - a linked list containing all the macro names for checking duplicates
 *      error - error flag
 * Returns: 1 if valid, 0 otherwise
 */

int label_name_valid(char *name, int LC, macro_Linked_list *macro_table, int *error) {
    int i = 0;
    macro_Linked_list *tmp = macro_table;
    char no_colon_string[LINE_LENGTH];
    int instruction_amount = 18; /* 16 instructions + mcro + mcroend */
    int register_amount = 8; /* r0-r7 */
    int length;
    const char *known_instructions[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn", "rts", "stop", "mcro", "mcroend"};
    const char *all_registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    if (name == NULL || name[0] == '\0') {
        return 0;
    }
    strncpy(no_colon_string, name, LINE_LENGTH - 1);
    no_colon_string[LINE_LENGTH - 1] = '\0';
    length = (int) strlen(no_colon_string);
    if (length > 0) {
        if (no_colon_string[length - 1] == '\n') {
            no_colon_string[length - 1] = '\0';
            length--;
        }
        if (no_colon_string[length - 1] == ':') {
            no_colon_string[length - 1] = '\0';
            length--;
        }
    }
    else {
        *error = 1;
        fprintf(stderr, "Error in line %d: empty string given.", LC);
        return 0;
    }

    /* check label name is not a valid macro name */
    while (tmp != NULL) {
        if (strcmp(tmp->name, no_colon_string) == 0) {
            fprintf(stderr, "Error in line %d: The label name is a name of a macro so you can't use it.\n", LC);
            *error = 1;
            return 0;
        }
        tmp = tmp->next_macro;
    }

    if (length > MAX_LABEL_NAME) {
        fprintf(stderr, "Error in line %d: The label name is longer than 30 chars.\n", LC);
        *error = 1;

        return 0;
    }
    if (isalpha(no_colon_string[0]) == 0) {
        fprintf(stderr, "Error in line %d: The first char in name is a number.\n", LC);
        *error = 1;
        return 0;
    }
    for (i = 0; no_colon_string[i] != '\0' && no_colon_string[i] != '\n'; i++) {
        if (isalnum((unsigned char)no_colon_string[i]) == 0 && no_colon_string[i] != '_') {
            fprintf(stderr, "Error in line %d: The char in the %dth position in label name is not alphanumeric.\n"
                , LC, i);
            *error = 1;
            return 0;
        }
    }
    for (i = 0; i < instruction_amount; i++) {
        if (strcmp(no_colon_string, known_instructions[i]) == 0) { // if label name is known instruction name.
            fprintf(stderr, "Error in line %d: The label name is a known instruction name (%s).\n", LC,
                known_instructions[i]);
            *error = 1;
            return 0;
        }
    }
    for (i = 0; i < register_amount; i++) {
        if (strcmp(no_colon_string, all_registers[i]) == 0) { // if label name is known instruction name.
            fprintf(stderr, "Error in line %d: The label name is a register (%s).\n", LC,
                all_registers[i]);
            *error = 1;
            return 0;
        }
    }
    return 1;
}

/*
 * Checks whether a given start of a string is a symbol that needs special addressing.
 * Params:
 *      first_word - the suspected symbol
 *      data_without_label_flag - a flag to check whether the given symbol is without a label
 *      LC - the current line
 *      macro_table - a linked list containing all the macro names for checking duplicates
 *      error - error flag
 * Returns: 1 if valid, 0 otherwise
 */
int is_symbol(char first_word[LINE_LENGTH], int *data_without_label_flag, int LC, macro_Linked_list *macro_table, int *error) {
    int i = 0;
    int length = (int) strlen(first_word);
    int possible_instruction_count = 5;
    const char *possible_instructions[] = {".data", ".string", ".mat", ".entry", ".extern"};
    if (length >= 2 && first_word[length - 1] == ':') {
        if (label_name_valid(first_word, LC, macro_table, error)) {
            return 1;
        }
    }
    for (i = 0; i < possible_instruction_count; i++) { // check if it's a call for data with no label
        if (strcmp(first_word, possible_instructions[i]) == 0) {
            *data_without_label_flag = 1;
            return 1;
        }
    }
    return 0;

}

/*
 * Checks whether a token is a valid instruction.
 * Params:
 *      first_word - the candidate word
 *      LC - current line
 *      error - error flag, set to 1 if not a valid instruction
 * Returns: 1 if valid, 0 otherwise
 */
int is_instruction(char first_word[LINE_LENGTH], int LC, int *error) {
    const char *known_instructions[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr",
        "red", "prn", "rts", "stop", "mcro", "mcroend", ".entry", ".extern"};
    if (first_word != NULL) {
        int i = 0;
        for (i = 0; i < 20; i++) {
            if (strcmp(first_word, known_instructions[i]) == 0) {
                return 1;
            }
        }
    }
    fprintf(stderr, "Error in line %d: unknown instruction '%s' in line. \n", LC, first_word);
    *error = 1;
    return 0;

}

// checks if a given first word or second word is data storing

int is_data_storing(char first_word[LINE_LENGTH], char second_word[LINE_LENGTH]) {
    char *data_storers[3] = {".data", ".string", ".mat"};
    int i = 0;
    if (second_word != NULL) {
        if (first_word[strlen(first_word) - 1] == ':') {
            int j = 0;
            for (j = 0; j < 3; j++) {
                if (strcmp(second_word, data_storers[j]) == 0) {
                    return 1;
                }
            }
        }
    }
    // check for data without label case

    for (i = 0; i < 3; i++) {
        if (strcmp(first_word, data_storers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Inserts a new label into a list */

void insert_to_label(label_list *list, char label_name[LINE_LENGTH], const char *label_type, int *error, const int LC,
    const int DC) {
    // check if label name is in the list yet
    // checking each time for the next one - so i could add it at the end
    label_list *tmp = list;
    while (tmp->next_label != NULL) {
        if (strcmp(label_name, tmp->label_name) == 0) {
            *error = 1;
            fprintf(stderr, "Error in line %d: Can't name two labels in the same name.\n", LC);
            return;
        }
        tmp = tmp->next_label;
    }

    // means the final one does not exist
    // meaning "should" add name, doesn't throw error
    if (strcmp(label_name, tmp->label_name) != 0) {
        // insert into tmp the current label
        label_list *to_add = malloc(sizeof(label_list));
        if (!to_add) {
            *error = 1;
            fprintf(stderr, "Error in line %d: Can't allocate memory for the new label.\n", LC);
            return;
        }
        int label_name_length = (int) strlen(label_name);
        if (label_name_length == 0) {
            *error = 1;
            fprintf(stderr, "Error in line %d: The label name is empty.\n", LC);
            return;
        }
        if (label_name[label_name_length-1] == ':') {
            label_name[label_name_length-1] = '\0';
        }
        strcpy(to_add->label_name, label_name);
        strcpy(to_add->label_type, label_type);
        to_add->value = DC;
        to_add->next_label = NULL;
        tmp->next_label = to_add;
    }
}


/*
 * Identifies data and writes the encoded values into memory[].
 * Params:
 *      data_type - the type of the data (".data", ".string", or ".mat")
 *      data - the actual data
 *      DC - data counter; incremented by the number of words written
 *      error - error flag set to 1 on parse/semantic errors
 *      LC - current source line number
 * Returns: void
 */
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
    if (strcmp(full_data, "") == 0 && strcmp(data_type, ".mat") != 0) {
        *error = 1;
        fprintf(stderr, "Error in line %d: data definition with no data.\n", LC);
        return;
    }
    // clear spaces after the data the could screw with .string
    if (strcmp(data_type, ".string") == 0) {
        int len = (int) strlen(full_data);
        int w;
        for (w = len - 1; w >= 0; w--) {
            if (full_data[w] == '"') {
                full_data[w + 1] = '\0';
                break;
            }
            if (full_data[w] != ' ' && full_data[w] != '\n') {
                *error = 1;
                fprintf(stderr, "Error in line %d: Bad quotation marks.\n", LC);
                return;
            }
        }
        len = (int) strlen(full_data);
        if (full_data[0] != '"' || full_data[len - 1] != '"') {
            *error = 1;
            fprintf(stderr, "Error in line %d: Bad quotation marks.\n", LC);
            return;
        }
    }
    if (strcmp(data_type, ".data") == 0) { // if contaings numbers and such
        int len = (int)strlen(full_data);
        while (i < len+1) {
            if ((full_data[i] == ',' && full_data[i + 1] == ',') || // shnei psikim beretzef yaani
                (full_data[i] == ',' && i == 0) || // psik the first
                (full_data[i] == ',' && i + 2 == len) // psik the last :O
                ) {
                *error = 1;
                fprintf(stderr, "Error in line %d: Bad commas.\n", LC);
                return;
            }
            // new number and we know not two psikim next to each other
            if (full_data[i] == ',' || full_data[i] == '\0' || full_data[i] == '\n') {
                if (appeared_numbers) {
                    if (memory_pointer < MEMORY_SIZE) {
                        memory[memory_pointer] = num * sign;
                        memory_pointer++;
                    }
                    else {
                        *error = 1;
                        fprintf(stderr, "Error in line %d: Memory overflow.\n", LC);
                        return;
                    }

                    word_count++; // new mila
                    sign = 1;
                    num = 0;
                    appeared_space = 0;
                    appeared_numbers = 0;
                }
                else if (full_data[i] == ',') {
                    *error = 1;
                    fprintf(stderr, "Error in line %d: Bad commas.\n", LC);
                    return;
                }
            }
            // make sure the plus sign only appears next to a number or at the start of the expression :)
            // this is a weird expression but it just answers the requirement of this
            else if (full_data[i] == '+' || full_data[i] == '-') {
                if (i > 0 && full_data[i-1] != ',' && full_data[i-1] != ' ' && full_data[i-1] != '\t') {
                    *error = 1;
                    fprintf(stderr, "Error in line %d: Bad signs.\n", LC);
                    return;
                }
                if (!(full_data[i+1] >= '0' && full_data[i+1] <= '9')) {
                    *error = 1; fprintf(stderr,"Error in line %d: Bad signs.\n", LC); return;
                }
                if (full_data[i] == '-') {
                    sign = -1;
                }
                if (full_data[i] == '+') {
                    sign = 1; // although it doesn't matter really
                }
                /* without these lines it would've continued to process the sign */
                i++;
                continue;
            }
            // check if all are numbers
            // + and - signs give false positive so account for these tooooooooooooooo
            else if (full_data[i] == ' ' || full_data[i] == '\t') {
                appeared_space = 1;
            }
            else if ((full_data[i] < '0' || full_data[i] > '9') && full_data[i] != '+' && full_data[i] != '-') {
                *error = 1;
                fprintf(stderr, "Error in line %d: Non number.\n", LC);
                return;
            }
            else {
                if (appeared_space && appeared_numbers) {
                    *error = 1;
                    fprintf(stderr, "Error in line %d: Bad commas.\n", LC);
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
            if (i != 0 && full_data[i] != '"') {
                // printf("added to memory: %d", (unsigned char)full_data[i]);
                if (memory_pointer < MEMORY_SIZE) {
                    memory[memory_pointer] = (unsigned char)full_data[i];
                    memory_pointer++;
                }
                else {
                    *error = 1;
                    fprintf(stderr, "Error in line %d: Memory overflow.\n", LC);
                    return;
                }
            }
            i++;
        }
        memory[memory_pointer] = (unsigned char)'\0';
        memory_pointer++;
        // now i is the length of the string so i-1 is supposed to be "
        if (full_data[0] == '"' && full_data[i-1] == '"') { // check if valid corners
            word_count += i - 1; // minus two quotes + '\0'
            // printf("I would've added %d words\n", word_count);
        }
        else {
            *error = 1;
            fprintf(stderr, "Error in line %d: Bad quotation marks.\n", LC);
            return;
        }
    }
    if (strcmp(data_type, ".mat") == 0) {
        int first_number;
        int second_number;
        int memory_pointer_before = memory_pointer;
        if (sscanf(full_data, "[%d][%d]", &first_number, &second_number) == 2) {
            word_count += first_number * second_number;
        }
        i = 6;/* Magic numbers are usually not good but it fits exactly by [%d][%d] */
        while (i < (int)strlen(full_data)+1) {
            if ((full_data[i] == ',' && full_data[i + 1] == ',') || // shnei psikim beretzef yaani
                (full_data[i] == ',' && i == 0) || // psik the first
                (full_data[i] == ',' && i + 1 == (int) strlen(full_data)) // psik the last :O
                ) {
                *error = 1;
                fprintf(stderr, "Error in line %d: Bad commas.\n", LC);
                return;
            }
            // new number and we know not two psikim next to each other
            if (full_data[i] == ',' || full_data[i] == '\0' || full_data[i] == '\n') {
                if (appeared_numbers) {
                    if (memory_pointer < MEMORY_SIZE) {
                        memory[memory_pointer] = num * sign;
                        memory_pointer++;
                    }
                    else {
                        *error = 1;
                        fprintf(stderr, "Error in line %d: Memory overflow.\n", LC);
                        return;
                    }
                }
                sign = 1;
                num = 0;
                appeared_space = 0;
                appeared_numbers = 0;
            }
            // make sure the plus sign only appears next to a number or at the start of the expression :)
            // this is a weird expression but it just answers the requirement of this
            else if ((full_data[i] == '+' || full_data[i] == '-') &&
                ((full_data[i - 1] != ',' && full_data[i-1] != '\t' && full_data[i-1] != ' ')
                    || full_data[i + 1] == ' ' || full_data[i+1] == '\t') && i != 0) {
                *error = 1;
                fprintf(stderr, "Error in line %d: Bad signs.\n", LC);
                return;
            }
            // check if all are numbers
            // + and - signs give false positive so account for these tooooooooooooooo
            else if (full_data[i] == ' ' || full_data[i] == '\t') {
                appeared_space = 1;
            }
            else if ((full_data[i] < '0' || full_data[i] > '9') && full_data[i] != '+' && full_data[i] != '-') {
                *error = 1;
                fprintf(stderr, "Error in line %d: Non number.\n", LC);
                return;
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
                    fprintf(stderr, "Error in line %d: Bad spaces.\n", LC);
                    return;
                }
                appeared_numbers = 1;
                appeared_space = 0;
                num *= 10;
                num += full_data[i] - '0';
            }
        i++;
        }
        // this means that not all slots have been filled. for example
        // allocating a matrix with 4 spaces but only giving 2 parameters (or zero)
        if (memory_pointer - memory_pointer_before < word_count) {
            int j;
            for (j = memory_pointer; j < memory_pointer_before + word_count; j++) {
                if (j < MEMORY_SIZE) {
                    memory[j] = 0;
                }
                else {
                    *error = 1;
                    fprintf(stderr, "Error in line %d: Memory overflow.\n", LC);
                    return;
                }
            }
            memory_pointer = memory_pointer_before + word_count;
        }
        // printf("I would've added %d words\n", word_count);
    }
    //update DC
    *DC += word_count;
}

/*
 * this function is specifically for the immediate number
 * checks if all digits after the # are a number
 */

int is_number(const char *word) {
    int has_digit = 0; // digit flag
    int i = 1; // start from one because of #
    if (word[1] == '+' || word[1] == '-') { // if - or + ignore
        i = 2;
    }
    while (word[i] != '\0') {
        if (word[i] < '0' || word[i] > '9') {
            return 0;
        }
        i++;
        has_digit = 1;
    }
    return has_digit;
}

// checks if a number is in the register range

int is_register_range(char num) {
    if (num < '0' || num > '7') {
        return 0;
    }
    return 1;
}
/*
 * Analyzes the matrix using sscanf to be exactly like the format.
 * Params:
 *   op - the operand
 *   first_bracket + second_bracket - pointers to variables later that we will use to analyze the matrix further
 *   LC - current source line number
 *   error - flags error
 * Returns: if matrix is in format and it's brackets contacts into the pointers
 */
int analyze_matrix(char *op, char *first_bracket, char *second_bracket, int LC, int *error) {
    char matrix_label_name[LINE_LENGTH];
    char junk[LINE_LENGTH];
    // want to catch if there are stuff outside the matrix we don't want ! for example letters after the matrix and so on
    int catch = sscanf(op, "%[^[][%[^]]][%[^]]]%s", matrix_label_name, first_bracket, second_bracket, junk);
    if (catch == 3) {
        strcpy(op, matrix_label_name);
        return 1;
    }
    if (catch > 3) {
        *error = 1;
        fprintf(stderr, "Error in line %d: misdefinition of the matrix.\n", LC);
    }
    return 0;
}


/*
 * Analyzes a given operand and gives the addressing specifically for it.
 * Params:
 *   operand - the operand
 *   operand_type - addressing mode to set
 *   error - flags error
 *   register_flag - flag indicating if a register word was already counted (helps with the bits later)
 *   LC - current source line number
 * Returns: number of extra words contributed by this operand (0-2)
 */
int words_per_operand(char *operand, int *operand_type, int *error, int *register_flag, int LC) {
    // check if immediate
    // using fillers for the anlyze matrix function
    char first_bracket[LINE_LENGTH];
    char second_bracket[LINE_LENGTH];
    char matrix_tmp[LINE_LENGTH];
    if (operand[0] == '#') {
        if (operand[1] == '\0') {
            *error = 1;
            fprintf(stderr, "Error in line %d: Empty operand.\n", LC);
            return 0;
        }
        if (is_number(operand)) {
            //printf("line number %d added 1 word\n", LC);
            *operand_type = IMMEDIATE_CODE;
            return 1;
        }
        *error = 1;
        fprintf(stderr, "Error in line %d: Non number.\n", LC);
        return 0;
    }
    // check if register
    if (operand[0] == 'r' && (operand[2] == '\0' || operand[2] == '\n')) {
        if (is_register_range(operand[1])) {
            if (*register_flag == 0) {
                *register_flag = 1;
                *operand_type = REGISTER_CODE;
                //printf("line number %d added 1 word\n", LC);
                return 1;
            }
            *operand_type = REGISTER_CODE;
            return 0;
        }
    }

    strcpy(matrix_tmp, operand);
    if (analyze_matrix(matrix_tmp, first_bracket, second_bracket, LC, error)) {
        // idk if this is common c practice
        size_t first_len = strlen(first_bracket);
        size_t second_len = strlen(second_bracket);
        if (first_len == 2 && second_len == 2 && first_bracket[0] == 'r' && second_bracket[0] == 'r') {
            if (is_register_range(first_bracket[1]) && is_register_range(second_bracket[1])) {
                *operand_type = MATRIX_CODE;
                return 2;
            }
        }
        fprintf(stderr, "Error in line %d: misdefinition of the matrix.\n", LC);
        *error = 1;
        return 0;
    }
    // in this case, the first operand is not a matrix, direct, or register.
    // there are two possible scenarios - it's a label or it's junk
    // for now, let's assume it's a label and in the second maavar we will check if it's ok or not
    // i'm passing NULL here for macro table not the end of the world if it doesn't pass because in the label resolution
    // it will fall so i'm not concernd
    if (label_name_valid(operand, LC, NULL, error)){
        *operand_type = DIRECT_CODE;
        return 1; // in the case of a label
    }
    *error = 1;
    fprintf(stderr, "Error in line %d: bad operand usage.\n", LC);
    return 0;
}

/*
 * Analyzes operands and determines addressing modes and total word count.
 * Params:
 *   first_op - source operand
 *   second_op - destination operand
 *   source_mion - addressing mode of source operand
 *   dest_mion - addressing mode of destination operand
 *   error - error flag
 *   LC - current source line number
 * Returns: L (number of words for this instruction, including the opcode word)
 */
int analyze_operands(char *first_op, char *second_op, int *source_mion, int *dest_mion, int *error, int LC) {
    int L = 1;
    int found_register = 0; // because we want to make sure we only add one word for registers (in case there are two)
    // if command with one op
    if (!second_op) {
        source_mion = dest_mion;
    }
    // check first operand:
    if (first_op) {
        L += words_per_operand(first_op, source_mion, error, &found_register, LC);
    }
    if (second_op) {
        L += words_per_operand(second_op, dest_mion, error, &found_register, LC);
    }
    // if (!second_op && !first_op) {
    //     *error = 1;
    //     fprintf(stderr, "Error in line %d: misdefinition of the operand.\n", LC);
    //     return 0;
    // }

    return L;

}

/*
 * Builds the first 10-bit word: opcode + source/dest addressing bits.
 * Params:
 *   instruction - the mnemonic ("mov", "cmp", ...)
 *   source_mion - addressing mode of source operand
 *   dest_mion - addressing mode of destination operand
 * Returns: 10-bit encoded word
 */

unsigned short make_binary_line(const char *instruction, const int *first_op_type, const int *second_op_type) {
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
    const char *known_instructions[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne",
        "jsr", "red", "prn", "rts", "stop"};
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

/* Encodes an immediate operand into a 10-bit word.
 * Params:
 *   operand - operands like "#5", "#-12"
 * Returns: the 10-bit word: 10-bit two’s-complement value << IMMEDIATE_PADDING (ARE=00)
 */
unsigned short make_word(const char *operand) {
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
        if (operand[i] == ',' && operand[i+1] == '\0') { // i know it's lazy
            break;
        }
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

/* Encodes matrix brackets into the 10-bit matrix word (ARE left 00).
 * Params:
 *   first_bracket  - "r0".."r7" (row)
 *   second_bracket - "r0".."r7" (col)
 * Returns: the encoded 10-bit word with row at ROW_REGISTER_PADDING_FOR_MATRIX, col at COLUMN_REGISTER_PADDING_FOR_MATRIX
 */
unsigned short make_matrix_word(const char *first_bracket, const char *second_bracket) {
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

int contains_in_list(const char *str1, const char **list, int const size) {
    int i;
    for (i = 0; i<size; i++) {
        if (strcmp(str1, list[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/* this function checks that the instruction has enough operands
 * for example - the instruction mov requires two operands
 * this function will catch all the cases where an instruction requires n operands and make sure
 * the call for the instruction satisfies the operand requirements
 * returns 1 if call is INVALID and 0 if valid
 * Params:  instruction - the instruction
 *          first_op - the first operand
 *          second_op - the second operand
 *          source_mion - source addressing
 *          dest_mion - destination addressing
 * Returns: 1 if call is invalid, 0 otherwise
 */
int invalid_call(const char *instruction, const char *first_op, const char *second_op, const int *source_mion,
    const int *dest_mion) {

    const char *two_operands[] = {"mov", "cmp", "add", "sub", "lea"};
    const char *single_operand[] = { "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn"};
    const char *no_operands[] = {"rts", "stop"};

    const char *dest123[] = { "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "mov", "add", "sub", "lea"};

    /* expanding a bit on this function
     using page 47 from the book we can see the table that contains valid operations you can do with operands !
     i created the lists above according to the table and i will check them with for loops now here

     functions that take no source operands are always single operands so it caught it earlier
     also, i'm not checking for their source mov, cmp, add, sub. that's because they accept all types as source */

    if (strcmp(instruction, "lea") == 0 && (*source_mion == IMMEDIATE_CODE || *source_mion == REGISTER_CODE)) {
        return 1; // invalid call
    }
    if (contains_in_list(instruction, dest123, 12)) {
        /* this means operand can't have dest mion equal to 0*/
        if (*dest_mion == IMMEDIATE_CODE) {
            return 1; // invalid call
        }
    }

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

/*
*  Analyzes a single instruction and appends its binary words to the list that represents the lines in binary.
 * Params:
 *   L - number of words this instruction occupies
 *   line_binary_representation - the first word (opcode + addressing bits)
 *   immediate_word - extra word for immediates (if any)
 *   instruction - the instruction that needs to be analyzed
 *   first_op - source operand
 *   second_op - destination operand
 *   source_mion - addressing mode of source operand
 *   dest_mion - addressing mode of destination operand
 *   error - set to 1 on error
 *   LC - current source line number
 *   binary_list - pointer to append the built node
 *   IC - instruction counter (address of the first word)
 * Returns: void
 *
 * Note:
 * this is the most important function in the entire passage
 * like the name, the functions analyzes the instruction and then build the binary representation for it
 * basically, it crafts the representation for each word according to the book
 * and also puts the label words inside a special array
 * and there is an explanation about it from the header file about the data structure
 */

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
    if (!new_node) {
        *error = 1;
        fprintf(stderr, "Error in line %d: Memory allocation failed.\n", LC);
        return;
    }
    *L = analyze_operands(first_op, second_op, source_mion,
                            dest_mion, error, LC); // 13 dest and source nonsense fix it in the original function
    *line_binary_representation = make_binary_line(instruction, source_mion,
        dest_mion); // 14
    if (invalid_call(instruction, first_op, second_op, source_mion, dest_mion)) { // this means the call is invalid
        *error = 1;
        fprintf(stderr, "Error in line %d: The number of operands for the command in line is bad.\n", LC);
        free(new_node);
        return;
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
    // initialize the type for the labels
    for (i = 0; i < 2; i++) {
        new_node->labels[i] = "";
        new_node->labels_addressing[i] = 0;
    }
    // if immediate
    if (first_op != NULL && first_op[0] == '#') {
        // printf("this is immidiate\n");
        *immediate_word = make_word(first_op);
        new_node->words[1] = *immediate_word;
    }
    if (strcmp(instruction, "cmp") == 0 && *dest_mion == IMMEDIATE_CODE) {
        *immediate_word = make_word(second_op);
        new_node->words[2] = *immediate_word;
    }
    // very interesting check i want to do and didn't realise when i started this
    // check if the destination operand is 0!

    if (*source_mion == MATRIX_CODE) {
        analyze_matrix(first_op, first_bracket, second_bracket, LC, error);
        new_node->words[2] = make_matrix_word(first_bracket, second_bracket);
        new_node->labels[0] = new_strdup(first_op);
        new_node->labels_addressing[0] = 1; // words[1] needs to be updated to the location of the matrix
        //printf("sixth added %s intended to be placed in word place %d\n", new_node->labels[0], new_node->labels_addressing[0]);
    }
    if (*dest_mion == MATRIX_CODE) {
        analyze_matrix(second_op, first_bracket, second_bracket, LC, error);
        matrix_word = make_matrix_word(first_bracket, second_bracket);
        if (*source_mion == MATRIX_CODE) {
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
        }
    }
    if (first_op != NULL && *source_mion == REGISTER_CODE && first_op[1] != '\0') {
        source_r_number = first_op[1] - '0';
        // format it to 6-9
        source_r_number = source_r_number << REGISTER_SOURCE_PADDING;
        new_node->words[1] = source_r_number; //
    }
    if (second_op != NULL && *dest_mion == REGISTER_CODE && second_op[1] != '\0') {
        dest_r_number = second_op[1] - '0';
        // format it to 6-9
        dest_r_number = dest_r_number << REGISTER_DEST_PADDING;
        if (*source_mion == IMMEDIATE_CODE || *source_mion == DIRECT_CODE) {
            new_node->words[2] = dest_r_number; // words[0] is instruction words[1] is immediate
        }
        else if (*source_mion == MATRIX_CODE) { // if matrix
            new_node->words[3] = dest_r_number; // 0 is instruction, 1 is matrix, 2 is matrix registers 3 is destination
        }
        else if (*source_mion == REGISTER_CODE) {
            new_node->words[1] = new_node->words[1] | dest_r_number; // same extra word
        }
    }
    if (first_op != NULL && *dest_mion == REGISTER_CODE && second_op == NULL) { // very interesting case.
        dest_r_number = first_op[1] - '0';
        // format it to 6-9
        dest_r_number = dest_r_number << REGISTER_DEST_PADDING;
        new_node->words[1] = dest_r_number;
    }
    if (*source_mion == DIRECT_CODE) { // if label is source
        // now in the first passage words[1] is by default 0
        // so we update labels and labels addressing accordingly:
        new_node->labels[0] = new_strdup(first_op); // first operand is a label, "source"
        new_node->labels_addressing[0] = 1; // 0 is source and words[1] needs to be read as label
        //printf("first added %s intended to be placed in word place %d\n", new_node->labels[0], new_node->labels_addressing[0]);
    }
    if (*dest_mion == DIRECT_CODE) {
        if (*source_mion == MATRIX_CODE) { // if matrix is source and destination is label
            // because if for example jmp or any of the one operand instruction so the first op is the destination
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
            // because if for example jmp or any of the one operand instruction so the first op is the destination
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

/*
 * Iterates through all the complex structs and frees them
 */

void free_all(label_list *list, binary_line *line_to_free, macro_Linked_list *macro_table) {
    macro_Linked_list* iteratepoint;
    macro_Linked_list* trmp;
    while (line_to_free != NULL) {
        binary_line *tmp = line_to_free->next;
        // int i;
        // printf("\n");

        // for (i = 0; i<5; i++) {
        //     print_binary(line_to_free->words[i]);
        // }

        if (strcmp(line_to_free->labels[0], "") != 0) {
            free(line_to_free->labels[0]);
        }
        if (strcmp(line_to_free->labels[1], "") != 0) {
            free(line_to_free->labels[1]);
        }
        // printf("%d", line_to_free->LC);
        free(line_to_free);
        line_to_free = tmp;
    }
    free(line_to_free);
    // printf("\n");
    while (list->next_label != NULL) {
        label_list *tmp = list->next_label;
        // printf("%s | %d | %s\n", tmp->label_name, tmp->value, tmp->label_type);
        free(list);
        list = tmp;
    }
    free(list);

    iteratepoint = macro_table;
    while (iteratepoint != NULL) {
        Linked_List* iterate_linked = iteratepoint->first_instruction;
        while (iterate_linked!=NULL) {
            Linked_List* temp = iterate_linked->next_instruction;
            free(iterate_linked);
            iterate_linked = temp;
        }
        trmp = iteratepoint->next_macro;
        free(iteratepoint);
        iteratepoint = trmp;
    }
}

/* initial checks for each line
 * if it's a comment - skip
 * then it removes all spaces and checks for two commas next to each other
 */

void initial_error_checks(const char *line, int *error, int LC) {
    char line_without_spaces[LINE_LENGTH];
    int i;
    int z = 0;
    int w;
    int quotes_flag = 0;

    // comments
    w = 0;
    while (line[w] == ' ' || line[w] == '\t') {
        w++;
    }
    if (line[w] == ';' || line[w] == '\0' || line[w] == '\n') {
        return;
    }

    for (i = 0; i < LINE_LENGTH && line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
            line_without_spaces[z] = line[i];
            z++;
        }
    }
    line_without_spaces[z] = '\0';

    // check for two psikim next to eachother
    for (i = 0; i < z - 1; i++) {
        if (line_without_spaces[i] == '"') {
            quotes_flag = 1;
        }
        if (line_without_spaces[i] == ',' && line_without_spaces[i + 1] == ',' && quotes_flag == 0) {
            *error = 1;
            fprintf(stderr, "Error on line %d: Two psikim next to each other are not allowed.\n", LC);
        }
    }

}

/*
 * This function is the "master" function in the first passage (and because of the importance of the first passage)
 * it can be argued that this is one of the most important functions in the entire code.
 *
 * The function parses the post pre-assembler code and gives it a first and strong analysis by parsing labels and data
 * and all the other stuff.
 *
 * Furthermore, most of the error checking and validating comes here.
 *
 * Notice: this part doesn't generate any files (because of the skeleton definition) but it does make the foundation
 * (and even more than that) for building the output files.
 *
 * When runs into an error it doesn't stop the program completely but finishes reporting all errors and then exits
 * without triggering the second passage (as stated in the skeleton).
 */


void first_passage(macro_Linked_list *macro_list, char *post_file_name, char *original_argv) {
    int DC = 0;
    int IC = 100;
    int LC = 1; // line counter haha
    int exists_error = 0;
    int exists_label = 0;
    int w;
    char *first_word;
    char *second_word;
    char *third_word = 0;
    int data_without_label = 0; // in case there is data without a label (data wasting space for example .data)
    int DCF;
    int ICF;
    label_list *tmp;
    binary_line *instructions_iter;
    char line[LINE_LENGTH];
    FILE *source_asm = fopen(post_file_name, "r");
    if (!source_asm) {
        fprintf(stderr, "Error opening file %s\n", post_file_name);
        free_all(NULL, NULL, macro_list);
        return;
    }
    label_list* the_label_list = malloc(sizeof(label_list));
    if (!the_label_list) {
        fprintf(stderr, "Error: not enough memory\n");
        free_all(NULL, NULL, macro_list);
        fclose(source_asm);
        return;
    }
    binary_line* instructions_in_binary = malloc(sizeof(binary_line));
    if (!instructions_in_binary) {
        fprintf(stderr, "Error: not enough memory\n");
        free_all(the_label_list, NULL, macro_list);
        fclose(source_asm);
        return;
    }
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

    while (fgets(line, LINE_LENGTH, source_asm)) {

        /* immediately check for common errors*/
        initial_error_checks(line, &exists_error, LC);

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
        /* for the data without label case */
        if (strcmp(first_word, ".string") == 0 || strcmp(first_word, ".data") == 0) {
            second_word = strtok(NULL, "");
        }
        else { // jert
            second_word = strtok(NULL, " ,\t\n");
        }
        // printf("for the line %d the opps are first: %s;second: %s\n", LC, first_word, second_word);


        // this is for the case of wasting space in assembly and defining a line like this:
        // .data "somedata"
        // for some reason it's possible, in the project handbook i didn't see any refrence to it
        // but i guess i should take care of it too no?
        data_without_label = 0; // in case there is a data without label symbol (data wasting space for example .data)

        exists_label = is_symbol(first_word, &data_without_label, LC, macro_list, &exists_error);
        third_word = 0;
        if (!data_without_label) {
            if (second_word) {
                /* In case of string or data i just want the entire string to myself don't care about the rest */
                if (strcmp(second_word, ".string") == 0 || strcmp(second_word, ".data") == 0) {
                    third_word = strtok(NULL, "");
                }
                else {
                    third_word = strtok(NULL, " ,\t\n");
                }
            }
        }
        if (is_data_storing(first_word, second_word)) { // 5
            if (exists_label) {
                // in the case that the decleration is only .data, we will call without the name (first_word)

                if (data_without_label) {
                    identify_data(first_word, second_word, &DC, &exists_error, LC);
                }
                else {
                    insert_to_label(the_label_list, first_word, DATA_MARK, &exists_error, LC, DC); // 6

                    identify_data(second_word, third_word, &DC, &exists_error, LC); // 7
                }
            }
        }
        // non-data (entry and extern) are always data without label flag cases
        else if (data_without_label) { // if not data (line 8 // actually it is always here no need
            if (strcmp(first_word, ".entry") == 0) { // 9
                if (second_word == NULL) {
                    exists_error = 1;
                    fprintf(stderr, "Error in line %d: Called entry with no label on line.\n", LC);
                }
                else {
                    char *more = strtok(NULL, " ,\t\n");
                    if (more != NULL) {
                        exists_error = 1;
                        fprintf(stderr, "Error in line %d: Called entry with extra labels.\n", LC);
                    }
                }
                LC++;
                continue;
            }
            // if i was arrogant i would've left this if out of the code because if it reached here
            // and it is a symbol and clearly is not any of the other 4 options so it's for sure
            // insert_to_label(label_list *list, char label_name[LINE_LENGTH], int *error, int DC)
            if (strcmp(first_word, ".extern") == 0) {
                // 10
                if (second_word != NULL) {
                    char *more = strtok(NULL, " ,\t\n");
                    if (more != NULL) {
                        exists_error = 1;
                        fprintf(stderr, "Error in line %d: Called entry with extra labels.\n", LC);
                    }
                    else {
                        insert_to_label(the_label_list, second_word, EXTERNAL_MARK, &exists_error, LC, 0);
                    }
                }
                else {
                    exists_error = 1;
                    fprintf(stderr, "Error in line %d: Called extern with no label.\n", LC);
                }
            }

        }
        else if (second_word != NULL && strcmp(second_word, ".entry") == 0) { // i don't know how i didn't do it earlier
            LC++;
            continue;
        }
        else { // instruction without label ! // this means that first word has to be a known instruction
            unsigned short line_binary_representation;
            unsigned short immediate_word;
            int dest_mion = 0;
            int source_mion = 0;
            int L = 0;
            int len;

            if (exists_label) {
                insert_to_label(the_label_list, first_word, CODE_MARK, &exists_error, LC, IC); // 11
                if (second_word != NULL && is_instruction(second_word, LC, &exists_error)) { // 12
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
                        if (forth_word == NULL && third_word != NULL) {
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
                        fprintf(stderr, "Error in line %d: No psikim between operands.\n", LC);
                        exists_error = 1;
                    }
                    len = (int) strlen(line);
                    if (len >= 2 && line[strlen(line) - 2] == ',') {
                        fprintf(stderr, "Error in line %d: Psik for the last op why?.\n", LC);
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
                        if (second_word != NULL) {
                            strncpy(copy_second, second_word,LINE_LENGTH);
                            second_word = strtok(copy_second, " ,");
                            third_word = strtok(NULL, " ,");
                        }
                    }
                    analyze_and_build(&L, &line_binary_representation, &immediate_word, first_word, second_word,
                                            third_word, &source_mion, &dest_mion, &exists_error, LC, &instructions_iter, IC);
                }
                // if we have both a destination (third) and source (second) words and no psik in the line
                // means error
                // if a psik is somewhere else in the code, it will raise label not in list error

                if (number_of_psikim(line) == 0 && third_word != NULL && second_word != NULL) {
                    fprintf(stderr, "Error in line %d: No psikim between operands.\n", LC);
                    exists_error = 1;
                }
                if (line[strlen(line) - 2] == ',') {
                    fprintf(stderr, "Error in line %d: Psik for the last op why?.\n", LC);
                    exists_error = 1;
                }

            }
            // else {
            //
            // }
            IC += L;
        }

    LC++;
    }

    if (exists_error) { // 17
        // fflush(stderr);
        free_all(the_label_list, instructions_in_binary, macro_list);
        fclose(source_asm);
        return;
    }
    fclose(source_asm);

    DCF = DC;
    // printf("remember you did not use DCF %d", DCF);
    ICF = IC;

    tmp = the_label_list;

    while (tmp != NULL) {
        if (strcmp(tmp->label_type, DATA_MARK) == 0) {
            tmp->value = tmp->value + ICF;
            // printf("hi! ium %s and take %d\n",tmp->label_name, tmp->value);
        }
        tmp = tmp->next_label;
    }
    second_passage(instructions_in_binary, the_label_list, ICF, macro_list, post_file_name, original_argv);
}