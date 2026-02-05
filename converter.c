#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "toIntermediate.h"

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
    {"multf", 0x16},
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
            instructionLine |= (0x10 << shift);
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

            if (res == NULL) {
                free(fileCopy);
                return NULL;
            }
            for (int i = 0; i < count; i++) {
                free(values[i]);
            }

            fwrite(&res, sizeof(uint32_t), 1, fptr);
        } 
        token = strtok_r(NULL, "\n", &lineptr);
        first = 1;
    }
    fclose(fptr);

    free(fileCopy);
}

int deciVerify64(char* c) {
    if (c[0] == '-') {
        return 0;
    }
    long num = 0;

    for (int i = 0; c[i] != '\0'; i++) {
        if (c[i] - '0' >= 0 || c[i] - '0' <= 9) {
            if (num > (pow(2, 64) - 1) / 10) {
                perror("Num out of range!");
                return 0;
            } else {
                num = c[i] + num*10;
            }
            continue;
        } else {
            perror("Not a number!");
            return 0;
        }
    }
    return 1;
}

uint64_t getAddress(char* val, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(instructionLabel[i], val) == 0) {
            return instructionAddress[i];
        }
    }
    return 0;
}

// count for data too, increase by 8
char* expandLD(char* rd, char* value, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    uint64_t val;

    if (value == NULL) {
        perror("invalid value!\n");
        return NULL;
    }

    if (value[0] == ':') {
        val = getAddress(value, instructionLabel, instructionAddress, labelCount);

        if (val == 0) {
            perror("invalid label address!");
            return NULL;
        }
    } else if (deciVerify64(value)) { // // check this todo
        val = strtoull(value, NULL, 0);
    } else {
        perror("Invalid address for ld!!");
        return NULL;
    }

    char* result = malloc(2048);
    sprintf(result, "\txor %s, %s, %s\n", rd, rd, rd);

    char tmp[64];
    for (int shift = 52; shift >= 4; shift -= 12) {
        sprintf(tmp, "\taddi %s, %lu\n", rd, (val >> shift &0xFFF));
        // 52, 40, 28, 16, 4
        strcat(result, tmp);
        if (shift > 4) {
            sprintf(tmp, "\tshftli %s, 12\n", rd);
            strcat(result, tmp);
        }
    }
    sprintf(tmp, "\tshftli %s, 4\n", rd);
    strcat(result, tmp);
    sprintf(tmp, "\taddi %s, %lu\n", rd, (val & 0xF));
    strcat(result, tmp);
    return result;
}

char* expandPush(char* rd) {
    char* result = malloc(512);
    result[0] = '\0';

    sprintf(result, "\tmov (r31)(-8), %s\n", rd);
    strcat(result, "\tsubi r31, 8\n");
    return result;
}

char* expandPop(char* rd) {
    char* result = malloc(512);
    result[0] = '\0';

    char tmp[64];
    sprintf(tmp, "\tmov %s, (r31)(-8)\n", rd);
    strcat(result, tmp);
    strcat(result, "\taddi r31, 8\n");
    return result;
}

char* expandClr(char* rd) {
    char* result = malloc(256);
    sprintf(result, "\txor %s, %s, %s\n", rd, rd, rd);
    return result;
}

int verifyRegister(char* c) {
    if (strlen(c) == 0) {
        perror("invalid length register!\n");
        return 0;
    }

    if (strlen(c) <= 1) {
        return 0;
    }

    int first = 0;
    if (c[0] == 'r') {
        first++;
    }
    for (int i = 1; c[i] != '\0'; i++) {
        if (!(c[i] - '0' >= 0 && c[i] - '0' < 10)) {
            first--;
        } else {
            first++;
        }
    }
    int n = atoi(c + 1);

    if (n < 0 || n > 31) {
        perror("invalid register!\n");
        return 0;
    }

    return first > 1;
}

int deciVerify(char* c, int flag) {
    if (strlen(c) == 0) {
        perror("number given has a length of 0!");
        return 0;
    }
    if (!flag && c[0] == '-') {
        perror("Number is supposed to be unsigned!");
        return 0;
    }
    for (int i = 0; c[i] != '\0'; i++) {
        if ((c[i] - '0' >= 0 || c[i] - '0' <= 9) || (i == 0 && c[0] == '-')) {
            continue;
        } else {
            return 0;
        }
    }

    int num = atoi(c);

    if (flag) {
        if (num < 0 && abs(num) > pow(2, 11)) {
            return 0;
        } else if (num >= 0 && num > pow(2, 11) - 1) {
            return 0;
        }
    } else if (!flag && num > pow(2, 12) - 1) {
        return 0;
    }

    return 1;
}

char* getMacroLine(char* line, char** values, int count, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    char* result = malloc(512);

    char* opcode = values[0];

    // rd, rd, rd
    if (((strcmp(opcode, "add") == 0)
        || (strcmp(opcode, "sub") == 0)
        || (strcmp(opcode, "mul") == 0)
        || (strcmp(opcode, "div") == 0)
        || (strcmp(opcode, "addf") == 0)
        || (strcmp(opcode, "subf") == 0)
        || (strcmp(opcode, "mulf") == 0)
        || (strcmp(opcode, "divf") == 0)
        || (strcmp(opcode, "and") == 0)
        || (strcmp(opcode, "or") == 0)
        || (strcmp(opcode, "xor") == 0)
        || (strcmp(opcode, "brgt") == 0)
        || (strcmp(opcode, "shftr") == 0)
        || (strcmp(opcode, "shftl") == 0))
        && count == 4) {
            for (int i = 1; i < count; i++) {
                if (!verifyRegister(values[i])) {
                    perror("invalid register in instruction line!");
                    return NULL;
                }
            }

            sprintf(result, "%s\n", line);
            return result;
    }

    // rd, rd
    if ((strcmp(opcode, "brnz") == 0)
        || (strcmp(opcode, "not") == 0)) {
            for (int i = 1; i < count; i++) {
                if (!verifyRegister(values[i])) {
                    perror("invalid register in instruction line!");
                    return NULL;
                }
            }
            sprintf(result, "%s\n", line);
            return result;
    }
    
    // rd, L
    /*
    Unsigned literals: addi, subi, shftli, shftri, mov rd, L, ld
    Signed literals: brr, mov rd, (rs)(L), mov (rd)(L), rs
    */
    if (((strcmp(opcode, "addi") == 0)
        || (strcmp(opcode, "subi") == 0)
        || (strcmp(opcode, "shftri") == 0)
        || (strcmp(opcode, "shftli") == 0))
        && count == 3) {
            if (!verifyRegister(values[1]) || !deciVerify(values[2], 0)) {
                perror("invalid register or decimal value in instruction line!");
                return NULL;
            }
            sprintf(result, "%s\n", line);
            return result;
    }

    // rd
    if (((strcmp(opcode, "br") == 0)
    || (strcmp(opcode, "call") == 0))
    && count == 2) {
        if (!verifyRegister(values[1])) {
            perror("invalid register in instruction line!");
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    // L
    if (strcmp(opcode, "brr") == 0) {
        if (!deciVerify(values[1], 1)) {
            perror("invalid decimal value in instruction line!");
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    if (strcmp(opcode, "return") == 0 && count == 1) return line;

    // 5 arguments
    if (strcmp(opcode, "priv") == 0) {
        if (!verifyRegister(values[1]) || !verifyRegister(values[2]) || !verifyRegister(values[3]) || !deciVerify(values[4], 0)) {
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    // seperate macro case
    if (strcmp(opcode, "mov") == 0 && count == 4) {
        if (strncmp(line, "\tmov (", 6) == 0 && count == 4) {
            if (verifyRegister(values[1]) && deciVerify(values[2], 1) && verifyRegister(values[3])) {
                sprintf(result, "%s\n", line);
                return result;
            }
        } else {
            if (verifyRegister(values[1]) && verifyRegister(values[2]) && deciVerify(values[3], 1)) {
                sprintf(result, "%s\n", line);
                return result;
            }
        }
    } else if (strcmp(opcode, "mov") == 0 && count == 3) {
        if (verifyRegister(values[1]) && (verifyRegister(values[2]) || deciVerify(values[2], 0))) {
            sprintf(result, "%s\n", line);
            return result;
        } else {
            perror("Mov arguments invalid");
            return NULL;
        }
    }

    if (strcmp(opcode, "in") == 0 || strcmp(opcode, "out") == 0) {
        if (verifyRegister(values[1]) && (verifyRegister(values[2]))) {
            char* result = malloc(256);
            if (strcmp(opcode, "out") == 0) {
                sprintf(result, "\tpriv %s ,%s, r0, 4\n", values[1], values[2]);
            } else {
                sprintf(result, "\tpriv %s, %s, r0, 3\n", values[1], values[2]);
            }

            return result;
        } else {
            return NULL;
        }
    }

    if (strcmp(opcode, "clr") == 0) {
        if (verifyRegister(values[1])) {
            return expandClr(values[1]);
        } else {
            perror("invalid error!");
            return NULL;
        }
    }
    if (strcmp(opcode, "ld") == 0) {
        if (verifyRegister(values[1])) {
            return expandLD(values[1], values[2], instructionLabel, instructionAddress, labelCount);
        }
    }
    if (strcmp(opcode, "halt") == 0) {
        return "\tpriv r0,r0,r0,0\n";
    }

    // TODO
    if (strcmp(opcode, "push") == 0) {
        if (verifyRegister(values[1])) {
            return expandPush(values[1]);
        } else {
            perror("invalid register!\n");
        }
    }
    if (strcmp(opcode, "pop") == 0) {
        if (verifyRegister(values[1])) {
            return expandPop(values[1]);
        } else {
            perror("invalid register!\n");
        }
    }

    printf("line: %s\n", line);
    perror("non-existent instruction!");
    return NULL;
}

char* expandMacros(char* file, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    char* fileCopy = strdup(file);
    char* result = malloc(strlen(file) * 1024);
    result[0] = '\0';

    char *lineptr;
    char* token = strtok_r(fileCopy, "\n", &lineptr);
    char* values[4];

    char* mode = ".code";

    while (token != NULL) {
        if (strcmp(token, mode) != 0 || strlen(result) == 0) {
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

            char* res = getMacroLine(token, values, count, instructionLabel, instructionAddress, labelCount);

            if (res == NULL) {
                free(fileCopy);
                free(result);
                return NULL;
            }
            for (int i = 0; i < count; i++) {
                free(values[i]);
            }            
            strcat(result, res);
        } else {
            if (token[0] != ':') {
                strcat(result, token);
                strcat(result, "\n");
            }
        }

        token = strtok_r(NULL, "\n", &lineptr);
    }

    free(fileCopy);
    return result;
}


int macroSize(char* line) {
    if (strncmp(line, "\tld", 3) == 0) return 12;
    if (strncmp(line, "\tpush", 5) == 0) return 2;
    if (strncmp(line, "\tpop", 4) == 0) return 2;
    return 1;
}

void writeToFile(char* toWrite, char* name) {

    FILE *fptr;

    fptr = fopen(name, "w");

    if (fptr == NULL) {
        perror("Error: Could not open file.\n");
        return;
    }

    fputs(toWrite, fptr);

    fclose(fptr);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        perror("Error: invalid number of inputs!");
        return 1;
    }

    char* cleanedFile = cleanFile(argv[1]);

    char instructionLabel[1024][256];
    unsigned int instructionAddress[1024];
    int labelCount = findAddress(cleanedFile, instructionLabel, instructionAddress);
    
    char* result = expandMacros(cleanedFile, instructionLabel, instructionAddress, labelCount);

    if (result == NULL) {
        perror("Invalid macro!\n");
        return 1;
    }
    writeToFile(result, argv[2]);
    writeBinary(result, argv[3]);

    return 0;
}
