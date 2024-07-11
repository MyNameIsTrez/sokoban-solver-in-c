#include <ctype.h>
#include <math.h>
#include <stdbool.h>
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

typedef uint32_t u32;
typedef int64_t i64;

enum tile {
	FLOOR,
	WALL,
	BOX,
	STORAGE,
	STORED_BOX,
};

enum push_direction {
	pushing_up    = 0x1,
	pushing_down  = 0x2,
	pushing_left  = 0x4,
	pushing_right = 0x8,
};

struct move {
	size_t x;
	size_t y;
	enum push_direction direction;
};

static enum tile map[MAX_HEIGHT][MAX_WIDTH];

static size_t width = 0;
static size_t height = 0;

static i64 empty_storages = 0;

static size_t current_solve_calls;
static size_t total_solve_calls;

static size_t max_depth = 1;

static struct move path[MAX_PATH_LENGTH];
static size_t path_length;

static char *maps[MAX_MAPS];
static size_t map_depths[MAX_MAPS];
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
		case '*':
			return STORED_BOX;
	}
	abort();
}

static void print_area_stats(void) {
	printf("current_solve_calls: %zu\n", current_solve_calls);
	printf("total_solve_calls: %zu\n", total_solve_calls);
	printf("branching factor: %.2f\n", pow(current_solve_calls, 1.0/max_depth)); // O(branching_factor ^ depth)
	printf("'wasted' solve() calls on iterative deepening: %.2f%%\n\n", (double)(total_solve_calls - current_solve_calls) / total_solve_calls * 100);
}

static void print_map(void) {
	print_area_stats();
	printf("width: %zu\n", width);
	printf("height: %zu\n", height);
	printf("empty_storages: %zu\n", empty_storages);
	printf("path_length: %zu\n", path_length);
	printf("path:\n");
	for (size_t i = 0; i < path_length; i++) {
		struct move m = path[i];
		printf("Push %s (%zu,%zu)\n",
			m.direction == pushing_up ? "up" :
			m.direction == pushing_down ? "down" :
			m.direction == pushing_left ? "left" : "right"
			, m.x, m.y);
	}
	printf("depth: %zu\n", max_depth);
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			printf("%c", tile_to_char(map[y][x]));
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

static void solve(size_t x, size_t y, size_t depth, size_t top_left_index);

static void push_up(size_t x, size_t y, size_t depth, size_t top_left_index) {
	// printf("In push_up() at (%zu,%zu)\n", x, y);

	if (map[y][x] == BOX) {
		// printf("In push_up(), BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_up};
		map[y][x] = FLOOR;
		map[y-1][x] = map[y-1][x] == FLOOR ? BOX : STORED_BOX;
		if (map[y-1][x] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		if (map[y-1][x] == STORED_BOX) {
			empty_storages++;
		}
		map[y-1][x] = map[y-1][x] == BOX ? FLOOR : STORAGE;
		map[y][x] = BOX;
	} else if (map[y][x] == STORED_BOX) {
		// printf("In push_up(), STORED_BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_up};
		map[y][x] = STORAGE;
		map[y-1][x] = map[y-1][x] == FLOOR ? BOX : STORED_BOX;
		empty_storages++;
		if (map[y-1][x] == STORED_BOX) {
			empty_storages--;
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		empty_storages--;
		if (map[y-1][x] == STORED_BOX) {
			empty_storages++;
		}
		map[y-1][x] = map[y-1][x] == BOX ? FLOOR : STORAGE;
		map[y][x] = STORED_BOX;
	}
}

static void push_down(size_t x, size_t y, size_t depth, size_t top_left_index) {
	// printf("In push_down() at (%zu,%zu)\n", x, y);

	if (map[y][x] == BOX) {
		// printf("In push_down(), BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_down};
		map[y][x] = FLOOR;
		map[y+1][x] = map[y+1][x] == FLOOR ? BOX : STORED_BOX;
		if (map[y+1][x] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		if (map[y+1][x] == STORED_BOX) {
			empty_storages++;
		}
		map[y+1][x] = map[y+1][x] == BOX ? FLOOR : STORAGE;
		map[y][x] = BOX;
	} else if (map[y][x] == STORED_BOX) {
		// printf("In push_down(), STORED_BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_down};
		map[y][x] = STORAGE;
		map[y+1][x] = map[y+1][x] == FLOOR ? BOX : STORED_BOX;
		empty_storages++;
		if (map[y+1][x] == STORED_BOX) {
			empty_storages--;
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		empty_storages--;
		if (map[y+1][x] == STORED_BOX) {
			empty_storages++;
		}
		map[y+1][x] = map[y+1][x] == BOX ? FLOOR : STORAGE;
		map[y][x] = STORED_BOX;
	}
}

static void push_left(size_t x, size_t y, size_t depth, size_t top_left_index) {
	// printf("In push_left() at (%zu,%zu)\n", x, y);

	if (map[y][x] == BOX) {
		// printf("In push_left(), BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_left};
		map[y][x] = FLOOR;
		map[y][x-1] = map[y][x-1] == FLOOR ? BOX : STORED_BOX;
		if (map[y][x-1] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		if (map[y][x-1] == STORED_BOX) {
			empty_storages++;
		}
		map[y][x-1] = map[y][x-1] == BOX ? FLOOR : STORAGE;
		map[y][x] = BOX;
	} else if (map[y][x] == STORED_BOX) {
		// printf("In push_left(), STORED_BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_left};
		map[y][x] = STORAGE;
		map[y][x-1] = map[y][x-1] == FLOOR ? BOX : STORED_BOX;
		empty_storages++;
		if (map[y][x-1] == STORED_BOX) {
			empty_storages--;
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		empty_storages--;
		if (map[y][x-1] == STORED_BOX) {
			empty_storages++;
		}
		map[y][x-1] = map[y][x-1] == BOX ? FLOOR : STORAGE;
		map[y][x] = STORED_BOX;
	}
}

static void push_right(size_t x, size_t y, size_t depth, size_t top_left_index) {
	// printf("In push_right() at (%zu,%zu)\n", x, y);

	if (map[y][x] == BOX) {
		// printf("In push_right(), BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_right};
		map[y][x] = FLOOR;
		map[y][x+1] = map[y][x+1] == FLOOR ? BOX : STORED_BOX;
		if (map[y][x+1] == STORED_BOX) {
			empty_storages--;
			check_is_solved();
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		if (map[y][x+1] == STORED_BOX) {
			empty_storages++;
		}
		map[y][x+1] = map[y][x+1] == BOX ? FLOOR : STORAGE;
		map[y][x] = BOX;
	} else if (map[y][x] == STORED_BOX) {
		// printf("In push_right(), STORED_BOX is seen\n");

		path[path_length++] = (struct move){.x=x, .y=y, .direction=pushing_right};
		map[y][x] = STORAGE;
		map[y][x+1] = map[y][x+1] == FLOOR ? BOX : STORED_BOX;
		empty_storages++;
		if (map[y][x+1] == STORED_BOX) {
			empty_storages--;
		}

		solve(x, y, depth+1, top_left_index);

		path_length--;
		empty_storages--;
		if (map[y][x+1] == STORED_BOX) {
			empty_storages++;
		}
		map[y][x+1] = map[y][x+1] == BOX ? FLOOR : STORAGE;
		map[y][x] = STORED_BOX;
	}
}

static void flood(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]);

static void flood_up(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]) {
	// printf("In flood_up() at (%zu,%zu)\n", x, y);

	if (map[y-1][x] == FLOOR || map[y-1][x] == STORAGE) {
		// printf("Flooding up\n");
		flood(x, y-1, reachable, pushable);
	} else if ((map[y-1][x] == BOX || map[y-1][x] == STORED_BOX) && (map[y-2][x] == FLOOR || map[y-2][x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[y-2][x] == FLOOR && map[y-3][x] == WALL && (map[y-2][x-1] == WALL || map[y-2][x+1] == WALL)) {
			return;
		}

		// printf("Will be pushing box up\n");
		pushable[y-1][x] |= pushing_up;
	}
}

static void flood_down(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]) {
	// printf("In flood_down() at (%zu,%zu)\n", x, y);

	if (map[y+1][x] == FLOOR || map[y+1][x] == STORAGE) {
		// printf("Flooding down\n");
		flood(x, y+1, reachable, pushable);
	} else if ((map[y+1][x] == BOX || map[y+1][x] == STORED_BOX) && (map[y+2][x] == FLOOR || map[y+2][x] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[y+2][x] == FLOOR && map[y+3][x] == WALL && (map[y+2][x-1] == WALL || map[y+2][x+1] == WALL)) {
			return;
		}

		// printf("Will be pushing box down\n");
		pushable[y+1][x] |= pushing_down;
	}
}

static void flood_left(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]) {
	// printf("In flood_left() at (%zu,%zu)\n", x, y);

	if (map[y][x-1] == FLOOR || map[y][x-1] == STORAGE) {
		// printf("Flooding left\n");
		flood(x-1, y, reachable, pushable);
	} else if ((map[y][x-1] == BOX || map[y][x-1] == STORED_BOX) && (map[y][x-2] == FLOOR || map[y][x-2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[y][x-2] == FLOOR && map[y][x-3] == WALL && (map[y-1][x-2] == WALL || map[y+1][x-2] == WALL)) {
			return;
		}

		// printf("Will be pushing box left\n");
		pushable[y][x-1] |= pushing_left;
	}
}

static void flood_right(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]) {
	// printf("In flood_right() at (%zu,%zu)\n", x, y);

	if (map[y][x+1] == FLOOR || map[y][x+1] == STORAGE) {
		// printf("Flooding right\n");
		flood(x+1, y, reachable, pushable);
	} else if ((map[y][x+1] == BOX || map[y][x+1] == STORED_BOX) && (map[y][x+2] == FLOOR || map[y][x+2] == STORAGE)) {
		// If the box would get stuck in a wall corner, without being put in storage, the move is invalid
		if (map[y][x+2] == FLOOR && map[y][x+3] == WALL && (map[y-1][x+2] == WALL || map[y+1][x+2] == WALL)) {
			return;
		}

		// printf("Will be pushing box right\n");
		pushable[y][x+1] |= pushing_right;
	}
}

static void flood(size_t x, size_t y, bool reachable[MAX_HEIGHT][MAX_WIDTH], enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH]) {
	if (reachable[y][x]) {
		return;
	}
	reachable[y][x] = true;

	flood_up(x, y, reachable, pushable);
	flood_down(x, y, reachable, pushable);
	flood_left(x, y, reachable, pushable);
	flood_right(x, y, reachable, pushable);
}

static void stringify_map(size_t top_left_index) {
	map_string_length = 0;
	for (size_t y = 0; y < height; y++) {
		for (size_t x = 0; x < width; x++) {
			map_string[map_string_length++] = tile_to_char(map[y][x]);
		}
		map_string[map_string_length++] = '\n';
	}

	size_t n = top_left_index;
	if (n == 0) {
		map_string[map_string_length++] = '0';
	}
	while (n > 0) {
		map_string[map_string_length++] = '0' + (n % 10);
		n /= 10;
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

static void solve(size_t x, size_t y, size_t depth, size_t parent_top_left_index) {
	// printf("In solve() at (%zu,%zu)\n", x, y);

	current_solve_calls++;
	total_solve_calls++;

	if (depth > max_depth) {
		return;
	}

	// printf("parent_top_left_index: %zu\n", parent_top_left_index);
	stringify_map(parent_top_left_index);
	// printf("map_string:\n%s\n", map_string);

	u32 bucket_index = elf_hash(map_string) % MAX_MAPS;

	u32 i = buckets[bucket_index];

	while (true) {
		if (i == UINT32_MAX) {
			// printf("Memoizing map:\n%.*s\n", (int)map_string_length, map_string);
			maps[maps_size] = map_strings + map_strings_size;
			map_depths[maps_size] = depth;
			memcpy(map_strings + map_strings_size, map_string, map_string_length+1);
			map_strings_size += map_string_length+1;

			// If this map hasn't been seen before, memoize it
			chains[maps_size] = buckets[bucket_index];
			buckets[bucket_index] = maps_size++;

			break;
		}

		if (strcmp(map_string, maps[i]) == 0) {
			if (depth < map_depths[i]) {
				map_depths[i] = depth;
				break;
			} else {
				return; // Memoization, by stopping if the map_string has been seen before
			}
		}

		i = chains[i];
	}

	// print_map();

	static bool reachable[MAX_HEIGHT][MAX_WIDTH];
	memset(reachable, false, sizeof(reachable));

	enum push_direction pushable[MAX_HEIGHT][MAX_WIDTH];
	memset(pushable, 0, sizeof(pushable));

	flood(x, y, reachable, pushable);

	size_t top_left_index = 0;
	for (size_t py = 0; py < height; py++) {
		for (size_t px = 0; px < width; px++) {
			if (reachable[py][px]) {
				top_left_index = px + py * width;
				goto found_top_left;
			}
		}
	}
found_top_left:

	for (size_t py = 0; py < height; py++) {
		for (size_t px = 0; px < width; px++) {
			enum push_direction d = pushable[py][px];
			if (d != 0) {
				if (d & pushing_up) {
					// printf("Pushing box (%zu,%zu) up\n", px, py);
					push_up(px, py, depth, top_left_index);
					// printf("Reverting pushing box (%zu,%zu) up\n", px, py);
				}
				if (d & pushing_down) {
					// printf("Pushing box (%zu,%zu) down\n", px, py);
					push_down(px, py, depth, top_left_index);
					// printf("Reverting pushing box (%zu,%zu) down\n", px, py);
				}
				if (d & pushing_left) {
					// printf("Pushing box (%zu,%zu) left\n", px, py);
					push_left(px, py, depth, top_left_index);
					// printf("Reverting pushing box (%zu,%zu) left\n", px, py);
				}
				if (d & pushing_right) {
					// printf("Pushing box (%zu,%zu) right\n", px, py);
					push_right(px, py, depth, top_left_index);
					// printf("Reverting pushing box (%zu,%zu) right\n", px, py);
				}
			}
		}
	}
}

static void reset(void) {
	maps_size = 0;
	map_string_length = 0;
	map_strings_size = 0;
	current_solve_calls = 0;
	memset(buckets, UINT32_MAX, sizeof(buckets));
}

int main(void) {
	size_t player_x = 0;
	size_t player_y = 0;

	size_t n = 1;
	char *line = malloc(n);
	while (getline(&line, &n, stdin) > 0) {
		if (line[0] == '%' || isspace(line[0])) { // If this line is a comment or whitespace
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
		height++;
	}
	free(line);
	printf("player_x: %zu\n", player_x);
	printf("player_y: %zu\n", player_y);

	check_is_solved();

	// See https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search
	// max_depth = 29; {
	for (;; max_depth++) {
		printf("max_depth: %zu\n", max_depth);
		reset();
		solve(player_x, player_y, 1, player_x + player_y * width);
		print_area_stats();
	}

	printf("No solution was found :(\n");
	exit(EXIT_FAILURE);
}
