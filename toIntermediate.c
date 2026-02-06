#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "toIntermediate.h"


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
    for (int i = 0; ch[i] != '\0'; i++) {
        if (ch[i] == ' ' || ch[i] == '\t') {
            return 0;
        }
    }

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
            char* res = handleTabs(line);
            strcat(cleaned, res);
        } else if (line[0] == ';') {
            continue;
        } else if (line[0] == ':') {
            if (checkLabel(line)) {
                strcat(cleaned, line);
            } else {
                perror("Invalid label line!");
                return NULL;
            }
        } else {
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
                }
            }
        } else if (strcmp(mode, ".data") == 0) {
            if (line[0] == '\t') {
                current += 8;
            }
        }
        if (line[0] == ':') {
            strcpy(instructionLabel[labelNum], line);
            instructionAddress[labelNum] = current;
            labelNum++;
        }
        line = strtok(NULL, "\n");
    }
    return labelNum;
}