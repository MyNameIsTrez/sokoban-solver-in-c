#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEIGHT 420
#define MAX_WIDTH 420

char map[MAX_HEIGHT][MAX_WIDTH];

size_t width = 0;
size_t height = 0;

static void print_map(void) {
	printf("width: %zu\n", width);
	printf("height: %zu\n", height);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			printf("%c", map[y][x]);
		}
		printf("\n");
	}
}

int main(void) {
	size_t n = 1;
	char *line = malloc(n);
	getline(&line, &n, stdin); // The first line is always a comment
	while (getline(&line, &n, stdin) > 0) {
		size_t len = 0;
		while (line[len] != '\n') {
			map[height][len] = line[len];
			len++;
		}
		width = len > width ? len : width;
		height++;
	}
	free(line);

	print_map();
}
