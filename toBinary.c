#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>
#include "toBinary.h"

typedef struct {
    const char* name;
    unsigned int opcode;
} OpcodeMap;

static const OpcodeMap opcode_table[] = {
    {"add", 0x18},
    {"addi", 0x19},
    {"sub", 0x1a},
    {"subi", 0x1b},
    {"mul", 0x1c},
    {"div", 0x1d},
    {"and", 0x0},
    {"or", 0x1},
    {"xor", 0x2},
    {"not", 0x3},
    {"shftr", 0x4},
    {"shftri", 0x5},
    {"shftl", 0x6},
    {"shftli", 0x7},
    {"br", 0x8},
    {"brnz", 0xb},
    {"call", 0xc},
    {"return", 0xd},
    {"brgt", 0xe},
    {"priv", 0xf},
    {"addf", 0x14},
    {"subf", 0x15},
    {"mulf", 0x16},
    {"divf", 0x17}
};

uint32_t lookup_opcode(char *str) {
    for (int i = 0; i < sizeof(opcode_table)/sizeof(opcode_table[0]); i++) {
        if(strcmp(opcode_table[i].name, str) == 0) {
            return (uint32_t) opcode_table[i].opcode;
        }
    }
    return 0;
}

uint32_t getBinaryInstruction(char** values, int count) {
    uint32_t instructionLine = 0;
    int shift = 27;
    
    if (strcmp(values[0], "brr") == 0) {
        char* toConvert = values[1];
        int len = 12;

        if (toConvert[0] == 'r') {
            instructionLine |= (0x9 << shift);
        } else {
            instructionLine |= (0xa << shift);
        }
    } else if (strcmp(values[0], "mov") == 0) {
        if (count == 3) {
            if (values[2][0] == 'r') {
                instructionLine |= (0x11 << shift);
            } else {
                instructionLine |= (0x12 << shift);
            }
        } else if (count == 4) {
            if (values[2][0] == 'r') {
                instructionLine |= (0x10 << shift);
            } else {
                instructionLine |= (0x13 << shift);
            }
        }
    } else {
        uint32_t code = lookup_opcode(values[0]);
        instructionLine |= (code << shift);
    }
    shift -= 5;

    for (int i = 1; i < count; i++) {
        char* val = values[i];

        uint32_t num_val;
        int bits;

        if(val[0] == 'r') {
            num_val = atoi(val + 1);
            bits = 5;
        } else {
            num_val = atoi(val);
            bits = 12;
        }

        
        if (shift < 0) break;
        if (bits != 12) {
            shift -= bits;
            instructionLine |= ((num_val & ((1 << bits) - 1)) << (shift + bits));
        } else {
            instructionLine |= ((num_val & ((1 << bits) - 1)) );
        }
    }
    return instructionLine;
}

void writeBinary(char* file, char* filepath) {
    char* fileCopy = strdup(file);

    FILE *fptr;

    fptr = fopen(filepath, "wb");

    if (fptr == NULL) {
        perror("Error: Could not open file.\n");
        return;
    }

    char *lineptr;
    char* token = strtok_r(fileCopy, "\n", &lineptr);
    char* values[5];

    char* mode = ".code";
    int first = 0;

    while (token != NULL) {
        if (strcmp(token, mode) != 0 || first == 0) {
            if (strcmp(token, ".code") == 0) {
                mode = ".code";
            } else if (strcmp(token, ".data") == 0) {
                mode = ".data";
            }
        }
        if (strlen(token) > 0 && token[0] == '\t' && strcmp(mode, ".code") == 0) {
            char *macroptr;
            char* dup = strdup(token);
            char* part = strtok_r(dup, "\t, ()", &macroptr);
            int count = 0;
            
            while (part != NULL) {
                values[count] = strdup(part);
                count++;
                part = strtok_r(NULL, "\t, ()", &macroptr);
            }

            uint32_t res = getBinaryInstruction(values, count);

            for (int i = 0; i < count; i++) {
                free(values[i]);
            }

            fwrite(&res, sizeof(uint32_t), 1, fptr);
        }
        
        if (strlen(token) > 0 && token[0] == '\t' && strcmp(mode, ".data") == 0) {
            uint64_t res = atoi(token + 1);

            fwrite(&res, sizeof(uint64_t), 1, fptr);
        }
        token = strtok_r(NULL, "\n", &lineptr);
        first = 1;
    }
    fclose(fptr);

    free(fileCopy);
}