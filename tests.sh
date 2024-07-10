#!/bin/bash

# gcc main.c -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -Ofast -march=native
gcc main.c -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -fsanitize=address,undefined

if [[ $? -ne 0 ]]
then
	echo "Compilation failed"
	exit 1
fi

< maps/level_963.txt ./a.out
# < maps/up_twice.txt ./a.out
# < maps/down_twice.txt ./a.out
# < maps/left_twice.txt ./a.out
# < maps/right_twice.txt ./a.out
