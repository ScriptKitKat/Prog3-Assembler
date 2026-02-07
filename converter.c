#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>
#include "toIntermediate.h"
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

int main(int argc, char* argv[]) {
    if (argc != 4) {
        perror("Error: invalid number of inputs!");
        return 1;
    }

    char* result = toIntermediate(argv[1], argv[2]);

    if (result == NULL) {
        perror("Invalid macro!\n");
        return NULL;
    }
    writeBinary(result, argv[3]);
    return 0;
}
