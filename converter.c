/*
Pseudocode:
# merge consecutive .code and .data directives and ignore comments and check if an instruction has too many arguments, too few, invalid registers, invalid instruction types, etc.
String cleanedFiled = cleanFile(file)

# pass to find instruction location (reading in String & store it)
instructionLabel string[]
instructionLocation unsigned int[]
findLabels(cleanFile, instructionLable, instructionLocation) // fake hashmap?


# expand macros + replace instruction locations
String result = expandMacros(cleanedFile, instructionLabel, instructionLocation)

# write to intermediate file
write(result, "out.tk")

# translate intermediate file to a binary file
String binFile = translate("out.tk")
write (binfile, "out.tko")

# Hashy browny mappers??
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


// count for data too, increase by 8
char* expandMacros(char* file, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    char* fileCopy = strdup(file);  

    char* result = malloc(1024 * 1024);
    result[0] = '\0';

    char *lineptr;
    char* token = strtok_r(fileCopy, "\n", &lineptr);
    char* values[4];

    while (token != NULL) {
        if (strlen(token) > 0 && token[0] == '\t') {
            
            strcat(result, res);
            free(res);
            char *macroptr;
            char* part = strtok_r(token, "\t, ()", &macroptr);
            int count = 0;
            
            while (part != NULL) {
                if (count != 0) {
                    values[count - 1] = part;
                }
                count++;
                part = strtok_r(token, "\t, ()", &macroptr);
            }
            count--;

            char* res = getMacroLine(line, values, count, instructionLabel, instructionAddress, labelCount);

            if (res == NULL) {
                free(fileCopy);
                free(result);
                return NULL;
            }
        } else {
            strcat(result, token);
            strcat(result, "\n");
        }
        token = strtok_r(line, "\n", &lineptr);
    }

    free(fileCopy);
    return result;
}

int verifyRegister(char* c) {
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
        }
    }
    return first > 1;
}

/*
1) L can only be a floating point number for the ld macro, which means you would use the IEEE representation for L in this case.  Every other instruction treats L as 12 bits which doesn't allow for floating points.

2) Only if you're dealing with floating point numbers. IEEE is specifically for floating points so it wouldn't apply to integers. 

*/

int deciVerify(char* c, int flag) {
    if (!flag && c[0] == '-') {
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

    if (flag && abs(num) > pow(2, 11) - 1) {
        return 0;
    } else if (!flag && num > pow(2, 12) - 1) {
        return 0;
    }

    return 1;
}

/*
converting from integer to binary floating point form?
12 bit integer, unsigned: 2^12 - 1, signed:  -2^11 ~ 2^11 - 1

int deciVerify(char* c, int flag) {
    if (!flag && c[0] == '-') {
        return 0;
    }

    int deci = 0;
    for (int i = 0; c[i] != '\0'; i++) {
        if ((c[i] - '0' >= 0 || c[i] - '0' <= 9) || (i == 0 && c[0] == '-')) {
            continue;
        } else if (c[i] == '.') {
            deci++;
        } else {
            return 0;
        }
        if (deci > 1) {
            return 0;
        }
    }

    long c = decimalToNum(c);

    return c >= 0 ? 1 : 0;
}
*/

/*converting from decimal to floating point form*/
// long decimalToNum(char* c) {
//     long num = 0;

//     if (c[0] == '-') {

//     }
//     for (int i = 0; c[i] != '\0'; i++) {
//         int n = c[i] - '0';
        
//         num = num*10 + n;

//         if (num > pow(2, 12) - 1) {
//             perror("Value out of bound");
//             return -1;
//         }
//     }
//     return num;
// }

unsigned int getAddress(char* val, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(instructionLabel[i], val) == 0) {
            return instructionAddress[i];
        }
    }
    return -1;
}

char* getMacroLine(char* line, char** values, int count, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount) {
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
            return line;
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
            return line;
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
            return line;
    }

    // rd
    if (((strncmp(line, "\tbr", 3) == 0)
    || (strncmp(line, "\tcall", 5) == 0))
    && count == 1) {
        if (!verifyRegister(values[0])) {
            perror("invalid register in instruction line!");
            return NULL;
        }
        return line;
    }

    // L
    if (strncmp(line, "\tbrr", 4) == 0) {
        if (!deciVerify(values[1], 1)) {
            perror("invalid decimal value in instruction line!");
            return NULL;
        }
        return line;
    }

    if (strcmp(line, "\treturn") == 0 && count == 0) return line;

    // 5 arguments
    if (strncmp(line, "\tpriv", 5) == 0) {
        if (!verifyRegister(values[0]) || !verifyRegister(values[1]) || !verifyRegister(values[2]) || !deciVerify(values[3], 0)) {
            return NULL;
        }

        return line;
    }

    // seperate case
    if (strncmp(line, "\tmov", 4) == 0 && count == 3) {
        if (strncmp(line, "\tmov (", 6) == 0 && count == 3) {
            verifyRegister(values[0]);
            deciVerify(values[1], 1);
            verifyRegister(values[2]);
            return line;
        } else {
            verifyRegister(values[0]);
            verifyRegister(values[1]);
            deciVerify(values[2], 1);
        }
    } else if (strncmp(line, "\tmov", 4) == 0 && count == 2) {
        if (verifyRegister(values[0]) && (verifyRegister(values[1]) || deciVerify(values[1], 0))) {
            return line;
        } else {
            perror("Mov arguments invalid");
            return NULL;
        }
    }

    if (strncmp(line, "\tin", 3) == 0 || strncmp(line, "\tout", 4) == 0) {
        if (verifyRegister(values[0]) && (verifyRegister(values[1]))) {
            char* result = malloc(256);
            if (strncmp(line, "\tout", 4) == 0) {
                sprintf(result, "\tpriv %s,%s,r0,0x4\n", rd, rs);
            } else {
                sprintf(result, "\tpriv %s,%s,r0,0x3\n", rd, rs);
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
            return NULL;
        }
        return out;
    }

    if (strncmp(line, "\tld", 3) == 0) {
        if (verifyRegister(values[0])) {
            unsigned int val;
            if (values[1][0] == ':') {
                val = getAddress(val, instructionLabel, instructionAddress, labelCount);
                
                if (val == -1) {
                    perror("invalid address!");
                    return NULL;
                }
            } else if (deciVerify(values[1])) {
                val = decimalToNum(value[1]);
                // get the binary (unsigned int)
                // then bit shift mask to get the numbers :)
            } else {
                perror("Invalid address for ld!!");
                return NULL;
            }
            
            char* out = line;
            strcpy(out, "xor ");
            strcat(out, values[0]);
            strcat(out, ",");
            strcat(out, values[0]);
            strcat(out, ",");
            strcat(out, values[0]);
            strcat(out, "\naddi ");
            strcat(out, values[0]);
            strcat(out, ",");

            char* valToString;
            itoa(val>>52, valToString, 10);
            strcat(out, valToString);
        }
    }

    if (strcmp(line, "\thalt") == 0) {
        return "\tpriv r0,r0,r0,0x0\n";
    }

    if (strncmp(line, "\tpush", 5) == 0) {

        
    }
    if (strncmp(line, "\tpop", 4) == 0) {

    }

    perror("non-existent instruction!");
    return NULL;
}

char* expandLD(char* rd, char* value, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount)
 {

 }

char* expandPush(char* rd) {
    char* result = malloc(512);
    sprintf(result, "\tmov (r31)(12),%s");
}

char* expandPop(char* rd) {

}

char* expandClr(char* rd) {
    char* result = malloc(256);
    sprintf(result, "\txor %s,%s,%s\n", rd, rd, rd);
    return result;
}
// first pass -> labels, 2nd pass: write to output
int findAddress(char* file, char instructionLabel[][256], unsigned int* instructionAddress) {
    char* line = strtok(file, "\n");
    int labelNum = 0;
    unsigned int current = 0x1000;

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
                    current += macroSize(line) * 4;
                } else if (line[0] == ':') {
                    strcpy(instructionLabel[labelNum], line);
                    instructionAddress[labelNum] = current;
                    labelNum++;
                }
            }
        } else if (strcmp(mode, ".data") == 0) {
            if (line[0] == '\t') {
                current += 8;
            }
        }
        line = strtok(NULL, "\n");
    }
    return labelNum;
}

int macroSize(char* line) {
    if (strncmp(line, "\tld", 3) == 0) return 12;
    if (strncmp(line, "\tpush", 5) == 0) return 2;
    if (strncmp(line, "\tpop", 4) == 0) return 2;
    return 1;
}

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

    char* cleaned = (char*) malloc(fileSize * sizeof(char));
    char *line = NULL;
    size_t len = 0;
    int read;
    char* mode = ".code\n";
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == '.') {
            if (strcmp(line, mode) != 0) {
                if (strcmp(line, ".code\n") == 0) {
                    strcat(cleaned, ".code\n");
                    mode = ".code\n";
                } else if (strcmp(line, ".data\n") == 0) {
                    strcat(cleaned, ".data\n");
                    mode = ".data\n";
                } else {
                    perror("Invalid directive!");
                    free(cleaned);
                    fclose(f);
                    return NULL;
                }
            }
        } else if (line[0] == '\t') {
            char* res = line;
            int flag = 0;

            while(*line) {
                if (*line != ' ') {
                    if(flag == 0) {
                        flag++;
                    }
                    *res++ = *line;

                } else if (flag == 1) {
                    flag++;
                } else if (flag == 2) {
                    *res++ = *line;
                    flag++;
                }
                line++;
            }
            *res = '\0';

            strcat(cleaned, res);
        } else if (line[0] == ';') {
            continue;
        } else if (line[0] == ':') {
            strcat(cleaned, line);
        } else {
            perror("Invalid Line!");
            printf("Line: %s", line);
            free(cleaned);
            fclose(f);
            return NULL;
        }
    }
    free(line);
    fclose(f);

    return cleaned;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("Error: invalid number of inputs!");
        return 1;
    }

    char* cleanedFile = cleanFile(argv[1]);

    char instructionLabel[1024][256];
    unsigned int instructionAddress[1024];
    int labelCount = findAddress(cleanedFile, instructionLabel, instructionAddress);

    char* result = expandMacros(cleanedFile, instructionLabel, instructionAddress, labelCount);


    return 0;
}
