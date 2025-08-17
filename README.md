# Assembler in C

## Project Description
Assembler in c built for a uni project in The Open University of Israel.
The final executable is an assembler that can assemble .as and bring them to the pre-linker stage (.ob).

---

## File Overview
- **master.c** - Manages the start of the code and running each .as file.
- **preasm.c** - Resolves the macros.
- **firstpassage.c** - First passage of the assembler, translates most of the assembly code (apart from label resolving) into the corresponding binary values.  
- **secondpassage.c** - Second passage of the assembler, resolves the label addressing and creates the output files. 
- **memory.c / memory.h** - Memory handling. 
- **Makefile** - Build automation.

---

## Assumptions:
- While building the project, I've made some assumptions based on my understanding of the project instructions.
- **Macro building stage**
  - **No empty macros** - The assembler assumes that the assembly file does not contain any empty macros. If the assembly file does contain empty macros, the assembler will throw an error and not continue.
  - **No macro without end** - The assembler assumes that the assembly file does not contain "mcro" statements without corresponding "mcroend". If the assembly file does contain them, the assembler will throw an error and not continue.
  - **No nested macros** - The assembler assumes that the assembly file does not contain nested macros.

- **First passage**
  - **Operands** - The first passage assumes that everything that isn't a register, a matrix or immediate is a label.
  - Example: r8 should throw an error (out of range register) but the assembler treats it as a valid label name (in that case, if r8 is not defined the second passage will throw an error.
  
- **Memory handling**
  - **Memory** - The memory is a 256 integer array defined globally (memory) with a global pointer (memory_pointer). The assembler checks for overflow when updating DC and throws an error in the case of trying to load into memory more than 256 words. As stated in the project's instructions, the memory is 256 integers and every data instruction (.data, .mat, .string) is loaded into the memory. The assembler doesn't load IC into the memory and keeps it in a seperate linked list different from the data segmant. An alternative represantation is to remove the variable memorypointer and just use DC and IC to point to the array.
  - **IC and DC** - IC starts from 100. DC starts from zero. The assembler doesn't check for IC overflow into DC (impossible because of the +100 seperation and if the memory passes it then memory overflow from the pointer).
  - **The writer of the assembly file knows the limitations host system** - Because every word is encoded into 10 bits the limitation for immediate is 2^7 (10 bit places - 2 for ARE and - 1 for two's complement). The assembler does not check for numbers exceeding this range and these will cause an overflow in the system (the assembler will pass through with garbage values).
  - **.string** - The string can contain ANY char in ascii between 32 and 126, including more quotes (", 34 in ascii). The assembler does this by removing any leading tabs and spaces ands find the last qoutation marks. Even if there is an uneven amount of quoatitons marks the assembler will count them as part of the string and encode them into memory.
  - **.mat** - The assembler assumes that if the assembly code doesn't contain enough integers for the defined matrix the rest is filled with zeros.
  - **.data** - The assembler assumes floating point numbers are forbidden and throws and error if found.
- ## Error handling
  - **Stop on error** - When the assembler enconters an error it stops at the end of the segmant (preasm, firstpassage, secondpassage). This means that if any errors where found in the preasm step it stops there, and doesn't continue to the first passage, and the same for the first passage.
  - **Second passage** - If errors where found in the second passage no files will be created. 
---
## Build Instructions
To compile the project run:

```bash
make
make clean
