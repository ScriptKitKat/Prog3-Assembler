#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "toIntermediate.h"
#include <regex.h>
#include <ctype.h>

int macroSize(char* line) {
    if (strncmp(line, "\tld", 3) == 0) return 12;
    if (strncmp(line, "\tpush", 5) == 0) return 2;
    if (strncmp(line, "\tpop", 4) == 0) return 2;
    return 1;
}

char* handleTabs(char* line) {
    char* src = malloc(sizeof(char) * strlen(line) * 2);
    int inOpcode = 0;
    int gotSpace = 0;

    src[0] = '\t';
    int index = 1;
    for (int i = 1; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            src[index++] = line[i];
            if (line[i] == ',') {
                gotSpace = 0;
            }
            if (inOpcode == 0) {
                inOpcode = 1;
            }
        } else if (inOpcode == 1) {
            inOpcode = 2;
        }
        if (inOpcode == 2 && !gotSpace) {
            src[index++] = ' ';
            gotSpace = 1;
        }
    }
    src[index] = '\0';

    return src;
}

int checkLabel(char* ch) {
    regex_t re;

    if (regcomp(&re, "^:[0-9A-Za-z_-]+\\s*$", REG_EXTENDED | REG_NOSUB) == 0) {
        if (regexec(&re, ch, 0, NULL, 0) == 0) {
            regfree(&re);
            return 1;
        }
    }
    regfree(&re);

    return 0;
}

char* trim(char* word) {
    if (word == NULL) {
        perror("invalid entry");
        return NULL;
    }
    char* res = word;
    char* src = word;
    while (*src) {
        if (!isspace(*src)) {
            *res++ = *src;
        }
        src++;
    }

    *res = '\0';

    return word;
}

// Removes comments and 
char* cleanFile(char* file) {
    if (strlen(file) <= 0) {
        perror("Invalid file name length");
        return NULL;
    }
    FILE* f = fopen(file, "r");

    if (f == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* cleaned = (char*) malloc(fileSize + 1);
    if (!cleaned) {
        perror("Malloc failed!");
        return NULL;
    }
    cleaned[0] = '\0';

    char *line = NULL;
    size_t len = 0;
    int read;
    char* mode = ".code\n";
    int code = 0;
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == '.') {
            if (strcmp(line, mode) != 0 || strlen(cleaned) == 0) {
                if (strcmp(line, ".code\n") == 0) {
                    code = 1;
                    strcat(cleaned, ".code\n");
                    mode = ".code\n";
                } else if (strcmp(line, ".data\n") == 0) {
                    code = 1;
                    strcat(cleaned, ".data\n");
                    mode = ".data\n";
                } else {
                    perror("Invalid directive!");
                    free(cleaned);
                    fclose(f);
                    return NULL;
                }
            }
            if (strcmp(line, ".code\n") == 0 && code == 0) {
                strcat(cleaned, ".code\n");
                code = 1;
            }
        } else if (line[0] == '\t') {
            char* res = handleTabs(line);
            strcat(cleaned, res);
        } else if (line[0] == ';') {
            continue;
        } else if (line[0] == ':') {
            if (checkLabel(line)) {
                trim(line);
                strcat(line, "\n");
                strcat(cleaned, line);
            } else {
                perror("Invalid label line!");
                return NULL;
            }
        } else {
            printf("%s", line);
            perror("Invalid Line!");
            free(cleaned);
            fclose(f);
            return NULL;
        }
    }

    free(line);
    fclose(f);
    if (code == 1) {
        return cleaned;
    } else {
        perror("No .code!");
        free(cleaned);
        return NULL;
    }
}

int findAddress(char* file, char instructionLabel[][256], unsigned int* instructionAddress) {
    char* fileCopy = strdup(file);
    char* line = strtok(fileCopy, "\n");
    int labelNum = 0;
    unsigned int codeCurrent = 0x2000;

    unsigned int dataCurrent = 0x10000;

    char* mode = ".code";

    while (line != NULL) {
        if (strcmp(line, ".code") == 0) {
            mode = ".code";
        }

        if (strcmp(line, ".data") == 0) {
            mode = ".data";
        }

        if (strcmp(mode, ".code") == 0) {
            if (strlen(line) != 0) {
                if (line[0] == '\t') {
                    codeCurrent += macroSize(line) * 4;
                }
            }
        } else if (strcmp(mode, ".data") == 0) {
            if (line[0] == '\t') {
                dataCurrent += 8;
            }
        }
        if (line[0] == ':') {
            strcpy(instructionLabel[labelNum], line);
            if (strcmp(mode, ".code") == 0) {
                instructionAddress[labelNum] = codeCurrent;
            }
            if (strcmp(mode, ".data") == 0) {
                instructionAddress[labelNum] = dataCurrent;
            }
            labelNum++;
        }
        line = strtok(NULL, "\n");
    }
    return labelNum;
}

int deciVerify64(char* c) {
    if (c[0] == '-' || c == NULL) {
        perror("invalid L value!");
        return 0;
    }
    unsigned long long num = 0;

    for (int i = 0; c[i] != '\0'; i++) {
        if (c[i] >= '0' && c[i] <= '9') {
            int digit = c[i] - '0';

            if (num > (UINT64_MAX - digit) / 10) {
                perror("Num out of range!");
                return 0;
            } else {
                num = (digit) + num*10;
                
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
        perror("Invalid ld!!");
        return NULL;
    }

    char* result = malloc(2048);
    sprintf(result, "\txor %s, %s, %s\n", rd, rd, rd);

    char tmp[64];
    for (int shift = 52; shift >= 4; shift -= 12) {
        sprintf(tmp, "\taddi %s, %lu\n", rd, (val >> shift &0xFFF));
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
    sprintf(tmp, "\tmov %s, (r31)(0)\n", rd);
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
    if (strlen(c) == 0 || c == NULL) {
        perror("invalid register!\n");
        return 0;
    }

    if (strlen(c) <= 1) {
        return 0;
    }

    if (c[0] == 'r') {
        for (int i = 1; c[i] != '\0'; i++) {
            if (!(c[i] - '0' >= 0 && c[i] - '0' < 10)) {
                perror("invalid register!\n");
                return 0; 
            }
        }
        int n = atoi(c + 1);
    
        if (n < 0 || n > 31) {
            perror("invalid register!\n");
            return 0;
        }
        return 1;
    }

    return 0;
}

int deciVerify(char* c, int flag) {
    if (strlen(c) == 0 || c == NULL) {
        perror("invalid number!");
        return 0;
    }

    if (!flag && c[0] == '-') {
        perror("Number is supposed to be unsigned!");
        return 0;
    }

    unsigned long long num = 0;

    for (int i = (c[0] == '-') ? 1 : 0; c[i] != '\0'; i++) {
        if ((c[i] >= '0' && c[i] <= '9')) {
            int digit = c[i] - '0';
            num = (digit) + num*10;

            if (flag) {
                if (c[0] == '-' && num > 2048) {
                    return 0;
                } else if (c[0] != '-' && num > 2047) {
                    return 0;
                }
            } else if (!flag && num > 4095) {
                return 0;
            } 
        } else {
            return 0;
        }
    }

    return 1;
}

int movCase(char* line) {
    regex_t re;

    // mov r1, (r2)(L)
    if (regcomp(&re, "^\\s*mov r[0-9]+\\s*,\\s*\\(r[0-9]+\\)\\(-?:?[0-9]+\\)\\s*$", REG_EXTENDED | REG_NOSUB) == 0) {
        if (regexec(&re, line, 0, NULL, 0) == 0) {
            regfree(&re);
            return 1;
        }
    }
    regfree(&re);

    if (regcomp(&re, "^\\s*mov \\(r[0-9]+\\)\\(-?:?[0-9]+\\)\\s*,\\s*r[0-9]+\\s*$", REG_EXTENDED | REG_NOSUB) == 0) {
        if (regexec(&re, line, 0, NULL, 0) == 0) {
            regfree(&re);
            return 2;
        }
    }
    regfree(&re);
    
    if (regcomp(&re, "^\\s*mov r[0-9]+,\\s*r[0-9]+\\s*$", REG_EXTENDED | REG_NOSUB) == 0) {
        if (regexec(&re, line, 0, NULL, 0) == 0) {
            regfree(&re);
            return 3;
        }
    }
    regfree(&re);
    
    if (regcomp(&re, "^\\s*mov r[0-9]+, :?[0-9]+\\s*$", REG_EXTENDED | REG_NOSUB) == 0) {
        if (regexec(&re, line, 0, NULL, 0) == 0) {
            regfree(&re);
            return 4;
        }
    }
    regfree(&re);

    return -1;
}

char* getMacroLine(char* line, char** values, int count, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    char* result = malloc(1024);

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
    if (((strcmp(opcode, "brnz") == 0)
        || (strcmp(opcode, "not") == 0)) && count == 3) {
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
            if (!verifyRegister(values[1]) || (!deciVerify(values[2], 0) && values[2][0] != ':')) {
                perror("invalid register or decimal value in instruction line!");
                return NULL;
            }

            if (deciVerify(values[2], 0)) {
                sprintf(result, "%s\n", line);
                return result;
            }

            if (values[2][0] == ':') {
                uint64_t val = getAddress(values[2], instructionLabel, instructionAddress, labelCount);
                if (val == 0) {
                    perror("invalid label address!");
                    return NULL;
                }

                sprintf(result, "\t%s %s, %lu\n", values[0], values[1], val);
                return result;
            }
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
    if (strcmp(opcode, "brr") == 0 && count == 2) {
        if (strncmp(line, "\tbrr r", 6) == 0) {
            if (!verifyRegister(values[1])) {
                perror("invalid register in instruction line!");
                return NULL;
            }
        } else {
            if (!deciVerify(values[1], 1) && values[1][0] != ':') {
                perror("invalid decimal value in instruction line!");
                return NULL;
            }

            if (values[1][0] == ':') {
                uint64_t val = getAddress(values[1], instructionLabel, instructionAddress, labelCount);
                if (val == 0) {
                    perror("invalid label address!");
                    return NULL;
                }

                sprintf(result, "\t%s %lu\n", values[0], val);
                return result;
            }
        }

        sprintf(result, "%s\n", line);
        return result;
    }

    if (strcmp(opcode, "return") == 0 && count == 1) {
        sprintf(result, "%s\n", line);
        return result;
    }

    // 5 arguments
    if (strcmp(opcode, "priv") == 0) {
        if (!verifyRegister(values[1]) || !verifyRegister(values[2]) || !verifyRegister(values[3]) || !deciVerify(values[4], 0)) {
            return NULL;
        }
        sprintf(result, "%s\n", line);
        return result;
    }

    if (strcmp(opcode, "mov") == 0) {
        int caseMov = movCase(line);
        if (count == 3) {
            if (caseMov == 3 && verifyRegister(values[1]) && verifyRegister(values[2])) {
                sprintf(result, "%s\n", line);
                return result;
            }
            if (caseMov == 4 && verifyRegister(values[1]) && deciVerify(values[2], 0)) {
                sprintf(result, "%s\n", line);
                return result;
            }

            if (caseMov == 4 && verifyRegister(values[1]) && values[2][0] == ':') {
                uint64_t val = getAddress(values[2], instructionLabel, instructionAddress, labelCount);
                if (val == 0) {
                    perror("invalid label address!");
                    return NULL;
                }
                sprintf(result, "\t%s %s, %lu\n", values[0], values[1], val);
                return result;
            }
        }
        if (count == 4) {
            if (caseMov == 2 && verifyRegister(values[1]) && deciVerify(values[2], 1) && verifyRegister(values[3])) {
                sprintf(result, "%s\n", line);
                return result;
            }

            if (caseMov == 2 && verifyRegister(values[1]) && values[2][0] == ':') {
                uint64_t val = getAddress(values[2], instructionLabel, instructionAddress, labelCount);
                if (val == 0) {
                    perror("invalid label address!");
                    return NULL;
                }
                sprintf(result, "\t%s (%s)(%lu), %s\n", values[0], values[1], val, values[3]);
                return result;
            }

            if (caseMov == 1 && verifyRegister(values[1]) && verifyRegister(values[2]) && deciVerify(values[3], 1)) {
                sprintf(result, "%s\n", line);
                return result;
            }

            if (caseMov == 1 && verifyRegister(values[1]) && verifyRegister(values[2]) && values[3][0] == ':') {
                uint64_t val = getAddress(values[3], instructionLabel, instructionAddress, labelCount);
                if (val == 0) {
                    perror("invalid label address!");
                    return NULL;
                }
                sprintf(result, "\t%s %s, (%s)(%lu)\n", values[0], values[1], values[2], val);
                return result;
            }
        }
    }

    if ((strcmp(opcode, "in") == 0 || strcmp(opcode, "out") == 0) && count == 3) {
        if (verifyRegister(values[1]) && (verifyRegister(values[2]))) {
            char* result = malloc(256);
            if (strcmp(opcode, "out") == 0) {
                sprintf(result, "\tpriv %s, %s, r0, 4\n", values[1], values[2]);
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
            perror("invalid clr register!");
            return NULL;
        }
    }
    if (strcmp(opcode, "ld") == 0 && count == 3) {
        if (verifyRegister(values[1])) {
            return expandLD(values[1], values[2], instructionLabel, instructionAddress, labelCount);
        } else {
            perror("invalid ld!");
            return NULL;
        }
    }
    if (strcmp(opcode, "halt") == 0) {
        if (count == 1) {
            return "\tpriv r0, r0, r0, 0\n";
        } else {
            perror("invalid halt!");
            return NULL;
        }
    }

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

    printf("%s\n", line);
    perror("non-existent instruction!");
    return NULL;
}

char* expandMacros(char* file, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    char* fileCopy = strdup(file);
    char* result = malloc(strlen(file) * 2048);
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
                if (count > 4) {
                    return NULL;
                }
                part = strtok_r(NULL, "\t, ()", &macroptr);
            }

            char* res = getMacroLine(token, values, count, instructionLabel, instructionAddress, labelCount);

            if (res == NULL || strlen(res) == 0) {
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
                if (strcmp(mode, ".data") == 0 && strlen(token) > 0 && token[0] == '\t') {
                    char* num = token + 1;
                    if (!deciVerify64(num)) {
                        free(fileCopy);
                        free(result);
                        return NULL;
                    }
                    strcat(result, token);
                    strcat(result, "\n");
                } else {
                    strcat(result, token);
                    strcat(result, "\n");
                }
            }
        }

        token = strtok_r(NULL, "\n", &lineptr);
    }

    free(fileCopy);
    return result;
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

char* toIntermediate(char* inFile, char* outFile) {
    char* cleanedFile = cleanFile(inFile);

    if (cleanedFile == NULL) {
        perror("Invalid!\n");
        return NULL;
    }
    
    char instructionLabel[1024][256];
    unsigned int instructionAddress[1024];
    int labelCount = findAddress(cleanedFile, instructionLabel, instructionAddress);
    
    char* result = expandMacros(cleanedFile, instructionLabel, instructionAddress, labelCount);

    if (result == NULL) {
        perror("Invalid macro!\n");
        return NULL;
    }
    writeToFile(result, outFile);

    return result;
}