Name: Priscilla Ye
EID: pjy263

How to run code:
hw3 ./input_filename ./intermediate_filename.tk ./output_filename.tko

How to run tests:
tests ./test.tk

# My Pseudocode:
Merge consecutive .code and .data directives and ignore comments and check if an instruction has too many arguments, too few, invalid registers, invalid instruction types, etc.
String cleanedFiled = cleanFile(file)

Pass to find instruction location (reading in String & store it)
instructionLabel string[]
instructionLocation unsigned int[]
findLabels(cleanFile, instructionLable, instructionLocation) // fake hashmap?


Expand macros + replace instruction locations
String result = expandMacros(cleanedFile, instructionLabel, instructionLocation)

Write to intermediate file
write(result, "out.tk")

Translate intermediate file to a binary file
String binFile = translate("out.tk")
write (binfile, "out.tko")
*/