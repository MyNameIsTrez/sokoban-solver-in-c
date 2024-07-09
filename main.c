#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEIGHT 420
#define MAX_WIDTH 420

enum tile {
	FLOOR,
	WALL,
	BOX,
	STORAGE,
	PLAYER,
};

enum tile map[MAX_HEIGHT][MAX_WIDTH];

size_t width = 0;
size_t height = 0;

size_t player_x;
size_t player_y;

static char tile_to_char(enum tile t) {
	switch (t) {
		case FLOOR:
			return ' ';
		case WALL:
			return '#';
		case BOX:
			return '$';
		case STORAGE:
			return '.';
		case PLAYER:
			return '@';
	}
	abort();
}

static enum tile char_to_tile(char c) {
	switch (c) {
		case ' ':
			return FLOOR;
		case '#':
			return WALL;
		case '$':
			return BOX;
		case '.':
			return STORAGE;
		case '@':
			return PLAYER;
	}
	abort();
}

static void print_map(void) {
	printf("width: %zu\n", width);
	printf("height: %zu\n", height);
	printf("player_x: %zu\n", player_x);
	printf("player_y: %zu\n", player_y);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			printf("%c", tile_to_char(map[y][x]));
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
			enum tile t = char_to_tile(line[len]);
			map[height][len] = t;
			if (t == PLAYER) {
				player_x = len;
				player_y = height;
			}
			len++;
		}
		width = len > width ? len : width;
		height++;
	}
	free(line);

	print_map();
}
