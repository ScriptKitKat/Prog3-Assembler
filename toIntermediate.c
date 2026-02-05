#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "toIntermediate.h"

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
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == '.') {
            if (strcmp(line, mode) != 0 || strlen(cleaned) == 0) {
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
            char* src = line;
            char* dst = line;

            int sawTab = 0;
            int inOpcode = 0;
            int gotSpace = 0;
            while(*src) {
                if (!sawTab) {
                    if (*src == '\t') {
                        *dst++ = *src;
                        sawTab = 1;
                    }
                } else if (inOpcode == 0) {
                    if (*src != ' ' && *src != '\t') {
                        *dst++ = *src;
                        inOpcode = 1;
                    }
                } else if (inOpcode == 1 && !gotSpace) {
                    if (*src == ' ') {
                        *dst++ = ' ';
                        gotSpace = 1;
                    } else if (*src != ' ' && *src != '\t') {
                        *dst++ = *src;
                    }
                } else {
                    if (*src != ' ' && *src != '\t') {
                        *dst++ = *src;
                    }
                }
                src++;
            }
            *dst = '\0';
            strcat(cleaned, line);
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