id: pjy263
name: Priscilla Ye

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

/*
nt macroSize(char* line) {
    if (strncmp(line, "\tadd", 4) == 0) return 1;
    if (strncmp(line, "\taddi", 5) == 0) return 1;
    if (strncmp(line, "\tsub", 4) == 0) return 1;
    if (strncmp(line, "\tsubi", 5) == 0) return 1;
    if (strncmp(line, "\tmul", 4) == 0) return 1;
    if (strncmp(line, "\tdiv", 4) == 0) return 1;

    if (strncmp(line, "\taddf", 5) == 0) return 1;
    if (strncmp(line, "\tsubf", 5) == 0) return 1;
    if (strncmp(line, "\tmulf", 5) == 0) return 1;
    if (strncmp(line, "\tdivf", 5) == 0) return 1;

    if (strncmp(line, "\tand", 4) == 0) return 1;
    if (strncmp(line, "\tor", 3) == 0) return 1;
    if (strncmp(line, "\txor", 4) == 0) return 1;
    if (strncmp(line, "\tnot", 4) == 0) return 1;

    if (strncmp(line, "\tbr", 3) == 0) return 1;
    if (strncmp(line, "\tbrr", 4) == 0) return 1;
    if (strncmp(line, "\tbrr", 4) == 0) return 1;
    if (strncmp(line, "\tbrnz", 5) == 0) return 1;
    if (strncmp(line, "\tcall", 5) == 0) return 1;
    if (strncmp(line, "\treturn", 7) == 0) return 1;
    if (strncmp(line, "\tbrgt", 5) == 0) return 1;

    if (strncmp(line, "\tpriv", 5) == 0) return 1;

    if (strncmp(line, "\tshftr", 6) == 0) return 1;
    if (strncmp(line, "\tshftri", 7) == 0) return 1;
    if (strncmp(line, "\tshftl", 6) == 0) return 1;
    if (strncmp(line, "\tshftli", 7) == 0) return 1;

    if (strncmp(line, "\tmov", 4) == 0) return 1;

    if (strncmp(line, "\tin", 3) == 0) return 1;
    if (strncmp(line, "\tout", 4) == 0) return 1;
    if (strncmp(line, "\tclr", 4) == 0) return 1;
    if (strncmp(line, "\tld", 3) == 0) return 12;
    if (strncmp(line, "\tpush", 5) == 0) return 2;
    if (strncmp(line, "\tpop", 4) == 0) return 2;

    return -1;
}
*/