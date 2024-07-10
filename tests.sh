#!/bin/bash

# gcc iddfs.c -lm -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -Ofast -march=native
# gcc iddfs.c -lm -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -fsanitize=address,undefined

gcc bfs.c -lm -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -Ofast -march=native
# gcc bfs.c -lm -Wall -Wextra -Werror -Wpedantic -Wshadow -Wfatal-errors -g -fsanitize=address,undefined

if [[ $? -ne 0 ]]
then
	echo "Compilation failed"
	exit 1
fi

# < maps/box_on_storage.txt ./a.out
# < maps/down_twice.txt ./a.out
# < maps/left_twice.txt ./a.out
< maps/level_963.txt ./a.out
# < maps/level_40862.txt ./a.out
# < maps/level_47601.txt ./a.out
# < maps/right_twice.txt ./a.out
# < maps/sokoban_on_storage.txt ./a.out
# < maps/up_twice.txt ./a.out
