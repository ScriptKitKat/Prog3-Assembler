#ifndef TOINTER_H
#define TOINTER_H

char* cleanFile(char* file);

int findAddress(char* file, char instructionLabel[][256], unsigned int* instructionAddress);

char* handleTabs(char* line);

int checkLabel(char* ch);

int macroSize(char* line);

char* trim(char* word);

int deciVerify64(char* c);

uint64_t getAddress(char* val, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount);

char* expandLD(char* rd, char* value, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount);

char* expandPush(char* rd);

char* expandPop(char* rd);

char* expandClr(char* rd);

int verifyRegister(char* c);

int deciVerify(char* c, int flag);

int movCase(char* line);

char* getMacroLine(char* line, char** values, int count, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount);

char* expandMacros(char* file, char instructionLabel[][256], unsigned int* instructionAddress, int labelCount);

void writeToFile(char* toWrite, char* name);

char* toIntermediate(char* inFile, char* outFile);

#endif // TOINTER_H