#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEIGHT 16
#define MAX_WIDTH 16

#define MAX_PATH_LENGTH 420420
#define MAX_MAPS 42420420
#define MAX_MAP_STRING_LENGTH 420420
#define MAX_MAP_STRINGS_CHARS 420420420
#define QUEUE_LENGTH 420420

typedef uint32_t u32;
typedef int64_t i64;

enum tile {
	FLOOR,
	WALL,
	BOX,
	STORAGE,
	STORED_BOX,
};

struct entry {
	enum tile map[MAX_HEIGHT][MAX_WIDTH];
	size_t player_x;
	size_t player_y;
	char *path;
	size_t empty_storages;
	size_t depth;
};

static enum tile map[MAX_HEIGHT][MAX_WIDTH];

static size_t width = 0;
static size_t height = 0;

static size_t player_x;
static size_t player_y;

static i64 empty_storages = 0;

static size_t entries_seen;

static char path[MAX_PATH_LENGTH];
static size_t path_length;

static char *maps[MAX_MAPS];
static size_t maps_size;

static char map_strings[MAX_MAP_STRINGS_CHARS];
static size_t map_strings_size;

static u32 buckets[MAX_MAPS];
static u32 chains[MAX_MAPS];

static struct entry queue[QUEUE_LENGTH];
static size_t queue_start_index = 0;
static size_t queue_end_index = 0;

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
		case '*':
			return STORED_BOX;
	}
	abort();
}

static void print_bfs_stats(size_t depth) {
	printf("entries_seen: %zu\n", entries_seen);
	printf("queue_start_index: %zu\n", queue_start_index);
	printf("queue_end_index: %zu\n", queue_end_index);
	// printf("queue_length: %zu\n", );
	printf("branching factor: %.2f\n", pow(entries_seen, 1.0/depth)); // O(branching_factor ^ depth)
}

static void print_map(size_t depth) {
	print_bfs_stats(depth);
	printf("width: %zu\n", width);
	printf("height: %zu\n", height);
	printf("player_x: %zu\n", player_x);
	printf("player_y: %zu\n", player_y);
	printf("empty_storages: %zu\n", empty_storages);
	printf("path_length: %zu\n", path_length);
	printf("path: '%.*s'\n", (int)path_length, path);
	printf("depth: %zu\n", depth);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			printf("%c", x == player_x && y == player_y ? '@' : tile_to_char(map[y][x]));
		}
		printf("\n");
	}
	printf("\n");
}

static void check_is_solved(size_t depth) {
	if (empty_storages == 0) {
		printf("Solved!\n");
		print_map(depth);
		exit(EXIT_SUCCESS);
	}
}

static void enqueue(size_t depth) {
	struct entry e;

	memcpy(e.map, map, sizeof(map));
	e.player_x = player_x;
	e.player_y = player_y;
	path[path_length] = '\0';
	e.path = strdup(path);
	printf("Storing path '%s'\n", e.path);
	e.empty_storages = empty_storages;
	e.depth = depth;

	queue[queue_end_index++] = e;
	queue_end_index %= QUEUE_LENGTH;

	if (queue_start_index == queue_end_index) {
		fprintf(stderr, "The queue is full! You need to up the QUEUE_LENGTH #define\n");
		exit(EXIT_FAILURE);
	}
}

static void up(size_t depth) {
	if (map[player_y-1][player_x] == FLOOR || map[player_y-1][player_x] == STORAGE) {
		path[path_length++] = 'u';
		player_y--;

		enqueue(depth+1);

		path_length--;
		player_y++;
	} else if (map[player_y-1][player_x] == BOX && (map[player_y-2][player_x] == FLOOR || map[player_y-2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y-2][player_x] == FLOOR && map[player_y-3][player_x] == WALL && (map[player_y-2][player_x-1] == WALL || map[player_y-2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'U';
		map[player_y-1][player_x] = FLOOR;
		map[player_y-2][player_x] = map[player_y-2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y--;
		if (map[player_y-1][player_x] == STORED_BOX) {
			empty_storages--;
			check_is_solved(depth);
		}

		enqueue(depth+1);

		path_length--;
		player_y++;
		if (map[player_y-2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y-2][player_x] = map[player_y-2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y-1][player_x] = BOX;
	} else if (map[player_y-1][player_x] == STORED_BOX && (map[player_y-2][player_x] == FLOOR || map[player_y-2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y-2][player_x] == FLOOR && map[player_y-3][player_x] == WALL && (map[player_y-2][player_x-1] == WALL || map[player_y-2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'U';
		map[player_y-1][player_x] = STORAGE;
		map[player_y-2][player_x] = map[player_y-2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y--;
		empty_storages++;
		if (map[player_y-1][player_x] == STORED_BOX) {
			empty_storages--;
		}

		enqueue(depth+1);

		path_length--;
		player_y++;
		empty_storages--;
		if (map[player_y-2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y-2][player_x] = map[player_y-2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y-1][player_x] = STORED_BOX;
	}
}

static void down(size_t depth) {
	if (map[player_y+1][player_x] == FLOOR || map[player_y+1][player_x] == STORAGE) {
		path[path_length++] = 'd';
		player_y++;

		enqueue(depth+1);

		path_length--;
		player_y--;
	} else if (map[player_y+1][player_x] == BOX && (map[player_y+2][player_x] == FLOOR || map[player_y+2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y+2][player_x] == FLOOR && map[player_y+3][player_x] == WALL && (map[player_y+2][player_x-1] == WALL || map[player_y+2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'D';
		map[player_y+1][player_x] = FLOOR;
		map[player_y+2][player_x] = map[player_y+2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y++;
		if (map[player_y+1][player_x] == STORED_BOX) {
			empty_storages--;
			check_is_solved(depth);
		}

		enqueue(depth+1);

		path_length--;
		player_y--;
		if (map[player_y+2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y+2][player_x] = map[player_y+2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y+1][player_x] = BOX;
	} else if (map[player_y+1][player_x] == STORED_BOX && (map[player_y+2][player_x] == FLOOR || map[player_y+2][player_x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y+2][player_x] == FLOOR && map[player_y+3][player_x] == WALL && (map[player_y+2][player_x-1] == WALL || map[player_y+2][player_x+1] == WALL)) {
			return;
		}

		path[path_length++] = 'D';
		map[player_y+1][player_x] = STORAGE;
		map[player_y+2][player_x] = map[player_y+2][player_x] == FLOOR ? BOX : STORED_BOX;
		player_y++;
		empty_storages++;
		if (map[player_y+1][player_x] == STORED_BOX) {
			empty_storages--;
		}

		enqueue(depth+1);

		path_length--;
		player_y--;
		empty_storages--;
		if (map[player_y+2][player_x] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y+2][player_x] = map[player_y+2][player_x] == BOX ? FLOOR : STORAGE;
		map[player_y+1][player_x] = STORED_BOX;
	}
}

static void left(size_t depth) {
	if (map[player_y][player_x-1] == FLOOR || map[player_y][player_x-1] == STORAGE) {
		path[path_length++] = 'l';
		player_x--;

		enqueue(depth+1);

		path_length--;
		player_x++;
	} else if (map[player_y][player_x-1] == BOX && (map[player_y][player_x-2] == FLOOR || map[player_y][player_x-2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x-2] == FLOOR && map[player_y][player_x-3] == WALL && (map[player_y-1][player_x-2] == WALL || map[player_y+1][player_x-2] == WALL)) {
			return;
		}

		path[path_length++] = 'L';
		map[player_y][player_x-1] = FLOOR;
		map[player_y][player_x-2] = map[player_y][player_x-2] == FLOOR ? BOX : STORED_BOX;
		player_x--;
		if (map[player_y][player_x-1] == STORED_BOX) {
			empty_storages--;
			check_is_solved(depth);
		}

		enqueue(depth+1);

		path_length--;
		player_x++;
		if (map[player_y][player_x-2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x-2] = map[player_y][player_x-2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x-1] = BOX;
	} else if (map[player_y][player_x-1] == STORED_BOX && (map[player_y][player_x-2] == FLOOR || map[player_y][player_x-2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x-2] == FLOOR && map[player_y][player_x-3] == WALL && (map[player_y-1][player_x-2] == WALL || map[player_y+1][player_x-2] == WALL)) {
			return;
		}

		path[path_length++] = 'L';
		map[player_y][player_x-1] = STORAGE;
		map[player_y][player_x-2] = map[player_y][player_x-2] == FLOOR ? BOX : STORED_BOX;
		player_x--;
		empty_storages++;
		if (map[player_y][player_x-1] == STORED_BOX) {
			empty_storages--;
		}

		enqueue(depth+1);

		path_length--;
		player_x++;
		empty_storages--;
		if (map[player_y][player_x-2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x-2] = map[player_y][player_x-2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x-1] = STORED_BOX;
	}
}

static void right(size_t depth) {
	if (map[player_y][player_x+1] == FLOOR || map[player_y][player_x+1] == STORAGE) {
		path[path_length++] = 'r';
		player_x++;

		enqueue(depth+1);

		path_length--;
		player_x--;
	} else if (map[player_y][player_x+1] == BOX && (map[player_y][player_x+2] == FLOOR || map[player_y][player_x+2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x+2] == FLOOR && map[player_y][player_x+3] == WALL && (map[player_y-1][player_x+2] == WALL || map[player_y+1][player_x+2] == WALL)) {
			return;
		}

		path[path_length++] = 'R';
		map[player_y][player_x+1] = FLOOR;
		map[player_y][player_x+2] = map[player_y][player_x+2] == FLOOR ? BOX : STORED_BOX;
		player_x++;
		if (map[player_y][player_x+1] == STORED_BOX) {
			empty_storages--;
			check_is_solved(depth);
		}

		enqueue(depth+1);

		path_length--;
		player_x--;
		if (map[player_y][player_x+2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x+2] = map[player_y][player_x+2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x+1] = BOX;
	} else if (map[player_y][player_x+1] == STORED_BOX && (map[player_y][player_x+2] == FLOOR || map[player_y][player_x+2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[player_y][player_x+2] == FLOOR && map[player_y][player_x+3] == WALL && (map[player_y-1][player_x+2] == WALL || map[player_y+1][player_x+2] == WALL)) {
			return;
		}

		path[path_length++] = 'R';
		map[player_y][player_x+1] = STORAGE;
		map[player_y][player_x+2] = map[player_y][player_x+2] == FLOOR ? BOX : STORED_BOX;
		player_x++;
		empty_storages++;
		if (map[player_y][player_x+1] == STORED_BOX) {
			empty_storages--;
		}

		enqueue(depth+1);

		path_length--;
		player_x--;
		empty_storages--;
		if (map[player_y][player_x+2] == STORED_BOX) {
			empty_storages++;
		}
		map[player_y][player_x+2] = map[player_y][player_x+2] == BOX ? FLOOR : STORAGE;
		map[player_y][player_x+1] = STORED_BOX;
	}
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

static char *stringify_map(void) {
	static char map_string[MAX_MAP_STRING_LENGTH];
	static size_t map_string_length;

	map_string_length = 0;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			map_string[map_string_length++] = x == player_x && y == player_y ? '@' : tile_to_char(map[y][x]);
		}
		map_string[map_string_length++] = '\n';
	}
	map_string[map_string_length] = '\0';

	fprintf(stderr, "Stringified map:\n%s\n", map_string);

	return strdup(map_string);
}

static void solve(void) {
	while (queue_start_index != queue_end_index) {
		struct entry e = queue[queue_start_index++];
		queue_start_index %= QUEUE_LENGTH;

		memcpy(map, e.map, sizeof(map));
		player_x = e.player_x;
		player_y = e.player_y;
		char *map_string = stringify_map();
		fprintf(stderr, "Dequeued map:\n%s", map_string);
		path_length = strlen(e.path);
		memcpy(path, e.path, path_length);
		fprintf(stderr, "With path that is %zu steps long:\n'%.*s'\n\n", path_length, (int)path_length, path);
		empty_storages = e.empty_storages;
		size_t depth = e.depth;

		entries_seen++;

		u32 bucket_index = elf_hash(map_string) % MAX_MAPS;

		u32 i = buckets[bucket_index];

		while (1) {
			if (i == UINT32_MAX) {
				fprintf(stderr, "Memoizing map:\n%s\n", map_string);
				maps[maps_size] = map_strings + map_strings_size;
				memcpy(map_strings + map_strings_size, map_string, strlen(map_string)+1);
				map_strings_size += strlen(map_string)+1;

				// If this map hasn't been seen before, memoize it
				chains[maps_size] = buckets[bucket_index];
				buckets[bucket_index] = maps_size++;

				break;
			}

			if (strcmp(map_string, maps[i]) == 0) {
				printf("Already memoized path '%s'\n", path);
				goto memoized; // Memoization, by stopping if the map_string has been seen before
			}

			i = chains[i];
		}

		up(depth);
		down(depth);
		left(depth);
		right(depth);

memoized:
		free(map_string);
		free(e.path);
	}
}

int main(void) {
	size_t n = 1;
	char *line = malloc(n);
	while (getline(&line, &n, stdin) > 0) {
		if (line[0] == '%') { // If this line is a comment
			continue;
		}

		size_t len = 0;
		while (line[len] != '\n') {
			char c = line[len];
			if (c == '@' || c == '+') {
				player_x = len;
				player_y = height;
				if (c == '+') {
					empty_storages++;
					map[height][len] = STORAGE;
				}
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
		if (width > MAX_WIDTH) {
			fprintf(stderr, "The map exceeds MAX_WIDTH\n");
			exit(EXIT_FAILURE);
		}
		height++;
		if (height > MAX_HEIGHT) {
			fprintf(stderr, "The map exceeds MAX_HEIGHT\n");
			exit(EXIT_FAILURE);
		}
	}
	free(line);

	print_map(1);
	check_is_solved(1);

	memset(buckets, UINT32_MAX, MAX_MAPS * sizeof(u32));
	enqueue(1);
	solve();
	print_bfs_stats(1);

	fprintf(stderr, "No solution was found :(\n");
	exit(EXIT_FAILURE);
}
