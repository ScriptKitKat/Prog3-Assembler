#!/usr/bin/sh

#set -x

gcc -g -Wall -Wextra converter.c toIntermediate.c -o hw3 -I include
