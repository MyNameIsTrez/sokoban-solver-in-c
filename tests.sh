#!/bin/bash

gcc main.c -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -fsanitize=address,undefined

< maps/level_963.txt ./a.out
