/*
 * Header file for the second passage.
 * Author: Shaul Joseph Sasson
 * Created 11/07/2025.
 *
 * Depends on preasm.h
 */

#ifndef SECONDPASSAGE_H
#define SECONDPASSAGE_H
#include <stdio.h>
#include "firstpassage.h"


void second_passage(binary_line *binary_line_list, label_list *the_label_list, int ICF, int DCF,
    macro_Linked_list *macro_table, char *file_name, char *original_argv);


#endif /* SECONDPASSAGE_H */
