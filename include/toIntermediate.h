#ifndef TOINTER_H
#define TOINTER_H

char* cleanFile(char* file);

int findAddress(char* file, char instructionLabel[][256], unsigned int* instructionAddress);

#endif // TOINTER_H