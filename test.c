#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <regex.h>
#include "toIntermediate.h"
#include "toBinary.h"
#include <assert.h>

void test_instruction(char* description, char** args, int count, uint32_t expected) {
    uint32_t result = getBinaryInstruction(args, count);
    printf("Testing: %-20s | Result: 0x%08X | Expected: 0x%08X ", description, result, expected);
    
    if (result == expected) {
        printf("[PASS]\n");
    } else {
        printf("[FAIL]\n");
    }
}

int checkIntermediate(char* result) {
    char* expectedOutput = 
    ".code\n"
	"\txor r5, r5, r5\n"
	"\taddi r5, 0\n"
	"\tshftli r5, 12\n"
	"\taddi r5, 0\n"
	"\tshftli r5, 12\n"
	"\taddi r5, 0\n"
	"\tshftli r5, 12\n"
	"\taddi r5, 0\n"
	"\tshftli r5, 12\n"
	"\taddi r5, 259\n"
	"\tshftli r5, 4\n"
	"\taddi r5, 4\n"
	"\tbr r5\n"
	"\tpriv r0, r0, r0, 0\n"
	"\txor r1, r1, r1\n"
	"\taddi r1, 0\n"
	"\tshftli r1, 12\n"
	"\taddi r1, 0\n"
	"\tshftli r1, 12\n"
	"\taddi r1, 0\n"
	"\tshftli r1, 12\n"
	"\taddi r1, 0\n"
	"\tshftli r1, 12\n"
	"\taddi r1, 262\n"
	"\tshftli r1, 4\n"
	"\taddi r1, 12\n"
	"\tbrnz r1, r2\n"
	"\tadd r0, r0, r1\n"
	"\txor r3, r3, r3\n"
	"\taddi r3, 0\n"
	"\tshftli r3, 12\n"
	"\taddi r3, 0\n"
	"\tshftli r3, 12\n"
	"\taddi r3, 0\n"
	"\tshftli r3, 12\n"
	"\taddi r3, 0\n"
	"\tshftli r3, 12\n"
	"\taddi r3, 266\n"
	"\tshftli r3, 4\n"
	"\taddi r3, 4\n"
	"\tbrgt r3, r5, r6\n"
	"\tpriv r0, r0, r0, 0\n";

    for (int i = 0; result[i] != '\0'; i++) {
        if (expectedOutput[i] == NULL || expectedOutput[i] != result[i]) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("Invalid number of arguments!");
        return 1;
    }

    int testsPassed = 0;

    char* result = toIntermediate(argv[1], "test_int.tk");

    if (checkIntermediate(result)) {
        testsPassed++;
    }

    printf("Whole file input test passed: %d/1\n", testsPassed);

    testsPassed = 0;
    // CHECK LABEL
    testsPassed += checkLabel(":l0");
    testsPassed += checkLabel(":l0 \n");

    // invalid cases
    testsPassed += checkLabel(":L 0\n") == 0 ? 1 : 0;
    testsPassed += checkLabel("eL 0\n") == 0 ? 1 : 0;

    printf("Check label test passed: %d/4\n", testsPassed);

    testsPassed = 0;
    if (strcmp(expandPush("r3"), "\tmov (r31)(-8), r3\n\tsubi r31, 8\n") == 0) {
        testsPassed++;
    }

    printf("Check push test passed: %d/1\n", testsPassed);

    testsPassed = 0;
    if (strcmp(expandPop("r3"), "\tmov r3, (r31)(0)\n\taddi r31, 8\n") == 0) {
        testsPassed++;
    }

    printf("Check pop test passed: %d/1\n", testsPassed);

    if (result == NULL) {
        perror("Invalid macro!\n");
        return NULL;
    }

    // Test binary
    char* case1[] = {"add", "r1", "r2", "r3"};
    test_instruction("add r1, r2, r3", case1, 4, 0xC0443000); 

    char* case2[] = {"addi", "r1", "10"};
    test_instruction("addi r1, 10", case2, 3, 0xC840000A);

    char* case3[] = {"brr", "5"};
    test_instruction("brr 5", case3, 2, 0x50000005);
    return 0;
}