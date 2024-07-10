#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEIGHT 420
#define MAX_WIDTH 420

#define MAX_PATH_LENGTH 420420
#define MAX_MAPS 420420
#define MAX_MAP_STRING_LENGTH 420420
#define MAX_MAP_STRINGS_CHARS 420420
#define MAX_BUCKETS 420420

typedef uint32_t u32;
typedef int64_t i64;

enum tile {
	FLOOR,
	WALL,
	BOX,
	STORAGE,
	STORED_BOX,
};

static enum tile map[MAX_HEIGHT][MAX_WIDTH];

static size_t width = 0;
static size_t height = 0;

static size_t player_x;
static size_t player_y;

static i64 empty_storages = 0;

static char path[MAX_PATH_LENGTH];
static size_t path_length;

static char *maps[MAX_MAPS];
static size_t maps_size;

static char map_string[MAX_MAP_STRING_LENGTH];
static size_t map_string_length;

static char map_strings[MAX_MAP_STRINGS_CHARS];
static size_t map_strings_size;

static u32 buckets[MAX_MAPS];
static u32 chains[MAX_MAPS];

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
		case STORED_BOX:
			return '$';
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
	}
	abort();
}

static void print_map(void) {
	printf("width: %zu\n", width);
	printf("height: %zu\n", height);
	printf("player_x: %zu\n", player_x);
	printf("player_y: %zu\n", player_y);
	printf("empty_storages: %zu\n", empty_storages);
	printf("path: '%.*s'\n", (int)path_length, path);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			printf("%c", x == player_x && y == player_y ? '@' : tile_to_char(map[y][x]));
		}
		printf("\n");
	}
	printf("\n");
}

static void check_is_solved(void) {
	if (empty_storages == 0) {
		printf("Solved!\n");
		print_map();
		exit(EXIT_SUCCESS);
	}
}

static void solve(void);

static void up(void) {
	if (map[player_y-1][player_x] == FLOOR || map[player_y-1][player_x] == STORAGE) {
		path[path_length++] = 'u';
		player_y--;
		solve();
		path_length--;
		player_y++;
	} else if (map[player_y-1][player_x] == BOX && (map[player_y-2][player_x] == FLOOR || map[player_y-2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y-2][player_x] == FLOOR && map[player_y-3][player_x] == WALL && (map[player_y-2][player_x-1] == WALL || map[player_y-2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'u';
		map[player_y-1][player_x] = FLOOR;
		map[player_y-2][player_x] = map[player_y-2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y--;
		if (map[player_y-1][player_x] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve();

		path_length--;
		player_y++;
		if (map[player_y-2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y-2][player_x] = map[player_y-2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y-1][player_x] = BOX;
	}
}

static void down(void) {
	if (map[player_y+1][player_x] == FLOOR || map[player_y+1][player_x] == STORAGE) {
		path[path_length++] = 'd';
		player_y++;
		solve();
		path_length--;
		player_y--;
	} else if (map[player_y+1][player_x] == BOX && (map[player_y+2][player_x] == FLOOR || map[player_y+2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y+2][player_x] == FLOOR && map[player_y+3][player_x] == WALL && (map[player_y+2][player_x-1] == WALL || map[player_y+2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'd';
		map[player_y+1][player_x] = FLOOR;
		map[player_y+2][player_x] = map[player_y+2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y++;
		if (map[player_y+1][player_x] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve();

		path_length--;
		player_y--;
		if (map[player_y+2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y+2][player_x] = map[player_y+2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y+1][player_x] = BOX;
	}
}

static void left(void) {
	if (map[player_y][player_x-1] == FLOOR || map[player_y][player_x-1] == STORAGE) {
		path[path_length++] = 'l';
		player_x--;
		solve();
		path_length--;
		player_x++;
	} else if (map[player_y][player_x-1] == BOX && (map[player_y][player_x-2] == FLOOR || map[player_y][player_x-2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x-2] == FLOOR && map[player_y][player_x-3] == WALL && (map[player_y-1][player_x-2] == WALL || map[player_y+1][player_x-2] == WALL)) {
			return;
		}

		path[path_length++] = 'l';
		map[player_y][player_x-1] = FLOOR;
		map[player_y][player_x-2] = map[player_y][player_x-2] == FLOOR ? BOX : STORED_BOX;
		player_x--;
		if (map[player_y][player_x-1] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve();

		path_length--;
		player_x++;
		if (map[player_y][player_x-2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x-2] = map[player_y][player_x-2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x-1] = BOX;
	}
}

static void right(void) {
	if (map[player_y][player_x+1] == FLOOR || map[player_y][player_x+1] == STORAGE) {
		path[path_length++] = 'r';
		player_x++;
		solve();
		path_length--;
		player_x--;
	} else if (map[player_y][player_x+1] == BOX && (map[player_y][player_x+2] == FLOOR || map[player_y][player_x+2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x+2] == FLOOR && map[player_y][player_x+3] == WALL && (map[player_y-1][player_x+2] == WALL || map[player_y+1][player_x+2] == WALL)) {
			return;
		}

		path[path_length++] = 'r';
		map[player_y][player_x+1] = FLOOR;
		map[player_y][player_x+2] = map[player_y][player_x+2] == FLOOR ? BOX : STORED_BOX;
		player_x++;
		if (map[player_y][player_x+1] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve();

		path_length--;
		player_x--;
		if (map[player_y][player_x+2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x+2] = map[player_y][player_x+2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x+1] = BOX;
	}
}

static void stringify_map(void) {
	map_string_length = 0;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			map_string[map_string_length++] = x == player_x && y == player_y ? '@' : tile_to_char(map[y][x]);
		}
		map_string[map_string_length++] = '\n';
	}
	map_string[map_string_length] = '\0';
}

// From https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=bfd/elf.c#l193
static u32 elf_hash(const char *namearg) {
	u32 h = 0;
	for (const unsigned char *name = (const unsigned char *) namearg; *name; name++) {
		h = (h << 4) + *name;
		h ^= (h >> 24) & 0xf0;
	}
	return h & 0x0fffffff;
}

static void solve(void) {
	stringify_map();

	u32 bucket_index = elf_hash(map_string) % MAX_BUCKETS;

	// fprintf(stderr, "bucket_index: %u\n", bucket_index);
	u32 i = buckets[bucket_index];
	// fprintf(stderr, "i == %u\n", i);

	while (1) {
		if (i == UINT32_MAX) {
			break;
		}

		// fprintf(stderr, "Comparing map:\n%.*s\n", (int)map_string_length, map_string);
		// fprintf(stderr, "with memoized map:\n%s\n", maps[i]);
		// abort();
		if (strcmp(map_string, maps[i]) == 0) {
			// abort();
			return; // Memoization, by returning if the map_string has been seen before
		}

		i = chains[i];
	}

	fprintf(stderr, "Memoizing map:\n%.*s\n", (int)map_string_length, map_string);
	maps[maps_size] = map_strings + map_strings_size;
	memcpy(map_strings + map_strings_size, map_string, map_string_length+1);
	map_strings_size += map_string_length+1;

	// If this map hasn't been seen before, memoize it
	chains[maps_size] = buckets[bucket_index];
	// fprintf(stderr, "buckets[%u] = %zu\n", bucket_index, maps_size);
	buckets[bucket_index] = maps_size++;

	up();
	down();
	left();
	right();
}

int main(void) {
	size_t n = 1;
	char *line = malloc(n);
	getline(&line, &n, stdin); // The first line is always a comment
	while (getline(&line, &n, stdin) > 0) {
		size_t len = 0;
		while (line[len] != '\n') {
			char c = line[len];
			if (c == '@') {
				player_x = len;
				player_y = height;
			} else {
				enum tile t = char_to_tile(c);
				if (t == STORAGE) {
					empty_storages++;
				}
				map[height][len] = t;
			}
			len++;
		}
		width = len > width ? len : width;
		height++;
	}
	free(line);

	print_map();

	memset(buckets, UINT32_MAX, MAX_BUCKETS * sizeof(u32));
	solve();

	fprintf(stderr, "No solution was found :(\n");
	exit(EXIT_FAILURE);
}
