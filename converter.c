#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "toIntermediate.h"

typedef struct {
    const char* name;
    char* opcode;
} OpcodeMap;

static const OpcodeMap opcode_table[] = {
    {"add", "11000"},
    {"addi", "11001"},
    {"sub", "11010"},
    {"subi", "11011"},
    {"mul", "11100"},
    {"div", "11101"},
    {"and", "00000"},
    {"or", "00001"},
    {"xor", "00010"},
    {"not", "00011"},
    {"shftr", "00100"},
    {"shftri", "00101"},
    {"shftl", "00110"},
    {"shftli", "00111"},
    {"br", "01000"},
    {"brnz", "01011"},
    {"call", "01100"},
    {"return", "01101"},
    {"brgt", "01110"},
    {"priv", "01111"},
    {"addf", "10010"},
    {"subf", "10101"},
    {"multf", "10110"},
    {"divf", "10111"}
};

char* lookup_opcode(char *str) {
    for (int i = 0; i < sizeof(opcode_table)/sizeof(opcode_table[0]); i++) {
        if(strcmp(opcode_table[i].name, str) == 0) {
            return opcode_table[i].opcode;
        }
    }

    return "";
}

char* convertToBinaryString(char* num, int size) {
    unsigned int binVal = atoi(num);
    char* result = malloc(512);
    result[0] = '\0';

    for(int shift = size - 1; shift >= 0; shift--) {
        if (binVal & (0x1 << shift)) {
            strcat(result, "1");
        } else {
            strcat(result, "0");
        }
    }

    return result;
}

char* getBinaryString(char** values, int count) {
    char* result = malloc(512);
    result[0] = '\0';
    
    if (strcmp(values[0], "brr") == 0) {
        char* toConvert = values[1];
        int len = 12;
        if (toConvert[0] == 'r') {
            strcat(result, "01001");
            toConvert++;
            len = 5;
        } else {
            strcat(result, "01010");
        }
        char* bin = convertToBinaryString(toConvert, len);
        strcat(result, bin);
    } else if (strcmp(values[0], "mov") == 0) {
        if (count == 3) {
            if (values[2][0] == 'r') {
                strcat(result, "10001");
                for (int i = 1; i < count; i++) {
                    char* toConvert = values[i];
                    int len = 12;
                    if (toConvert[0] == 'r') {
                        toConvert++;
                        len = 5;
                    }
                    char* bin = convertToBinaryString(toConvert, len);
                    strcat(result, bin);
                }
            } else {
                strcat(result, "10010");
                for (int i = 1; i < count; i++) {
                    char* toConvert = values[i];
                    int len = 12;
                    if (toConvert[0] == 'r') {
                        toConvert++;
                        len = 5;
                    }
                    char* bin = convertToBinaryString(toConvert, len);
                    strcat(result, bin);
                }
            }
        } else if (count == 4) {
            if (values[2][0] == 'r') {
                strcat(result, "10000");
                for (int i = 1; i < count; i++) {
                    char* toConvert = values[i];
                    int len = 12;
                    if (toConvert[0] == 'r') {
                        toConvert++;
                        len = 5;
                    }
                    char* bin = convertToBinaryString(toConvert, len);
                    strcat(result, bin);
                }
            } else {
                strcat(result, "10011");
                for (int i = 1; i < count; i++) {
                    char* toConvert = values[i];
                    int len = 12;
                    if (toConvert[0] == 'r') {
                        toConvert++;
                        len = 5;
                    }
                    char* bin = convertToBinaryString(toConvert, len);
                    strcat(result, bin);
                }
            }
        }
    } else {
        strcat(result, lookup_opcode(values[0]));
        for (int i = 1; i < count; i++) {
            char* toConvert = values[i];
            int len = 12;
            if (toConvert[0] == 'r') {
                toConvert++;
                len = 5;
            }
            char* bin = convertToBinaryString(toConvert, len);
            strcat(result, bin);
        }
    }

    while(32 - strlen(result) > 0) {
        strcat(result, "0");
    }
    return result;
}

char* getBinary(char* file) {
    char* fileCopy = strdup(file);
    char* result = malloc(strlen(file) * 1024);

    result[0] = '\0';
    char *lineptr;
    char* token = strtok_r(fileCopy, "\n", &lineptr);
    char* values[5];

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

            char* res = getBinaryString(values, count);

            if (res == NULL) {
                free(fileCopy);
                free(result);
                return NULL;
            }
            for (int i = 0; i < count; i++) {
                free(values[i]);
            }
            strcat(result, res);
        } 
        token = strtok_r(NULL, "\n", &lineptr);
    }

    free(fileCopy);
    return result;
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
    sprintf(result, "\txor %s,%s,%s\n", rd, rd, rd);

    char tmp[64];
    for (int shift = 52; shift >= 4; shift -= 12) {
        sprintf(tmp, "\taddi %s,%lu\n", rd, (val >> shift &0xFFF));
        strcat(result, tmp);
        if (shift > 4) {
            sprintf(tmp, "\tshftli %s,12\n", rd);
            strcat(result, tmp);
        }
    }

    sprintf(tmp, "\taddi %s,%lu\n", rd, (val & 0xF));
    strcat(result, tmp);
    sprintf(tmp, "\tshftli %s,4\n", rd);
    strcat(result, tmp);
    return result;
}

char* expandPush(char* rd) {
    char* result = malloc(512);
    sprintf(result, "\tmov (r31)(12),%s\n", rd);
    strcat(result, "\taddi r31,12\n");
    return result;
}

char* expandPop(char* rd) {
    char* result = malloc(512);
    result[0] = '\0';
    strcat(result, "\tsubi r31,12\n");
    char tmp[64];
    sprintf(tmp, "\tmov %s,(r31)(12)\n", rd);
    strcat(result, tmp);
    return result;
}

char* expandClr(char* rd) {
    char* result = malloc(256);
    sprintf(result, "\txor %s,%s,%s\n", rd, rd, rd);
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
            printf("here");
            return 0;
        }
    }

    int num = atoi(c);

    if (flag) {
        if (num < 0 && abs(num) > pow(2, 11)) {
            printf("here: %d", abs(num));
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
    // rd, rd, rd
    if (((strncmp(line, "\tadd", 4) == 0)
        || (strncmp(line, "\tsub", 4) == 0)
        || (strncmp(line, "\tmul", 4) == 0)
        || (strncmp(line, "\tdiv", 4) == 0)
        || (strncmp(line, "\taddf", 5) == 0)
        || (strncmp(line, "\tsubf", 5) == 0)
        || (strncmp(line, "\tmulf", 5) == 0)
        || (strncmp(line, "\tdivf", 5) == 0)
        || (strncmp(line, "\tand", 4) == 0)
        || (strncmp(line, "\tor", 3) == 0)
        || (strncmp(line, "\txor", 4) == 0)
        || (strncmp(line, "\tbrgt", 5) == 0)
        || (strncmp(line, "\tshftr", 6) == 0)
        || (strncmp(line, "\tshftl", 6) == 0))
        && count == 3) {
            for (int i = 0; i < count; i++) {
                if (!verifyRegister(values[i])) {
                    perror("invalid register in instruction line!");
                    return NULL;
                }
            }

            sprintf(result, "%s\n", line);
            return result;
    }

    // rd, rd
    if ((strncmp(line, "\tbrnz", 5) == 0)
        || (strncmp(line, "\tnot", 4) == 0)) {
            for (int i = 0; i < count; i++) {
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
    if (((strncmp(line, "\taddi", 5) == 0)
        || (strncmp(line, "\tsubi", 5) == 0)
        || (strncmp(line, "\tshftri", 7) == 0)
        || (strncmp(line, "\tshftli", 7) == 0))
        && count == 2) {
            if (!verifyRegister(values[0]) || !deciVerify(values[1], 0)) {
                perror("invalid register or decimal value in instruction line!");
                return NULL;
            }
            sprintf(result, "%s\n", line);
            return result;
    }

    // rd
    if (((strncmp(line, "\tbr", 3) == 0)
    || (strncmp(line, "\tcall", 5) == 0))
    && count == 1) {
        if (!verifyRegister(values[0])) {
            perror("invalid register in instruction line!");
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    // L
    if (strncmp(line, "\tbrr", 4) == 0) {
        if (!deciVerify(values[1], 1)) {
            perror("invalid decimal value in instruction line!");
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    if (strcmp(line, "\treturn") == 0 && count == 0) return line;

    // 5 arguments
    if (strncmp(line, "\tpriv", 5) == 0) {
        if (!verifyRegister(values[0]) || !verifyRegister(values[1]) || !verifyRegister(values[2]) || !deciVerify(values[3], 0)) {
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    // seperate macro case
    if (strncmp(line, "\tmov", 4) == 0 && count == 3) {
        if (strncmp(line, "\tmov (", 6) == 0 && count == 3) {
            if (verifyRegister(values[0]) && deciVerify(values[1], 1) && verifyRegister(values[2])) {
                sprintf(result, "%s\n", line);
                return result;
            }
        } else {
            if (verifyRegister(values[0]) && verifyRegister(values[1]) && deciVerify(values[2], 1)) {
                sprintf(result, "%s\n", line);
                return result;
            }
        }
    } else if (strncmp(line, "\tmov", 4) == 0 && count == 2) {
        if (verifyRegister(values[0]) && (verifyRegister(values[1]) || deciVerify(values[1], 0))) {
            sprintf(result, "%s\n", line);
            return result;
        } else {
            perror("Mov arguments invalid");
            return NULL;
        }
    }

    if (strncmp(line, "\tin", 3) == 0 || strncmp(line, "\tout", 4) == 0) {
        if (verifyRegister(values[0]) && (verifyRegister(values[1]))) {
            char* result = malloc(256);
            if (strncmp(line, "\tout", 4) == 0) {
                sprintf(result, "\tpriv %s,%s,r0,0x4\n", values[0], values[1]);
            } else {
                sprintf(result, "\tpriv %s,%s,r0,0x3\n", values[0], values[1]);
            }

            return result;
        } else {
            return NULL;
        }
    }

    if (strncmp(line, "\tclr", 4) == 0) {
        if (verifyRegister(values[0])) {
            return expandClr(values[0]);
        } else {
            perror("invalid error!");
            return NULL;
        }
    }
    if (strncmp(line, "\tld", 3) == 0) {
        if (verifyRegister(values[0])) {
            return expandLD(values[0], values[1], instructionLabel, instructionAddress, labelCount);
        }
    }
    if (strcmp(line, "\thalt") == 0) {
        return "\tpriv r0,r0,r0,0x0\n";
    }

    // TODO
    if (strncmp(line, "\tpush", 5) == 0) {
        if (verifyRegister(values[0])) {
            return expandPush(values[0]);
        }
    }
    if (strncmp(line, "\tpop", 4) == 0) {
        if (verifyRegister(values[0])) {
            return expandPop(values[0]);
        }
    }

    printf("non: %s", line);
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
                if (count > 0) {
                    values[count - 1] = strdup(part);
                }
                count++;
                part = strtok_r(NULL, "\t, ()", &macroptr);
            }
            count--;

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
    if (argc != 2) {
        perror("Error: invalid number of inputs!");
        return 1;
    }

    char* cleanedFile = cleanFile(argv[1]);

    printf("cleanedFile:\n%s", cleanedFile);
    char instructionLabel[1024][256];
    unsigned int instructionAddress[1024];
    int labelCount = findAddress(cleanedFile, instructionLabel, instructionAddress);
    
    char* result = expandMacros(cleanedFile, instructionLabel, instructionAddress, labelCount);
    writeToFile(result, "int.tk");

    char* bin = getBinary(result);
    writeToFile(bin, "output.tko");

    return 0;
}
