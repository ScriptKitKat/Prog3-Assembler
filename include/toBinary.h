#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>

uint32_t lookup_opcode(char *str);

uint32_t getBinaryInstruction(char** values, int count);

void writeBinary(char* file, char* filepath);