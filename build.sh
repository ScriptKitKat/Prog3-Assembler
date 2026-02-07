#!/usr/bin/sh

#set -x

gcc -g -Wall -Wextra converter.c toIntermediate.c toBinary.c -o hw3 -I include

gcc -g -Wall -Wextra test.c toIntermediate.c toBinary.c -o tests -I include

