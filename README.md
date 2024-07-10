# Sokoban solver in C

[Sokoban](https://en.wikipedia.org/wiki/Sokoban) is a game in which the player pushes boxes around in a warehouse, trying to get them to storage locations.

Writing a solver for it is similar to writing a chess engine, but simpler. Just like with chess, the branching factor (the number of possible moves on each turn) of a Sokoban level can get so ridiculously huge, that there are a practically infinite number of possible moves for complex Sokoban levels.

`bfs.c` ([breadth-first search](https://en.wikipedia.org/wiki/Breadth-first_search)) is best when the branching factor is near 1. The downside is that it runs out of memory in big maps.

`iddfs.c` ([iterative deepening depth-first search](https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search)) is best when the branching factor is quite a bit higher than 1. It doesn't run out of memory in big maps.

`area.c` is based on `iddfs.c`, but the major difference is that it doesn't make the player walk around one step at a time. Instead, it tracks which tiles are reachable by the player, so that it knows which boxes the player is able to push, if the player were to walk up to them.

## Running

`./tests.sh`

## Visualizing solutions

[Henry Kautz](https://henrykautz.com/sokoban/Sokoban.html) has a great website for visualizing Sokoban maps and solutions. The [help](https://henrykautz.com/sokoban/help.html) button at the bottom of that page explains the file format his website expects.

## Future plans

- Make version of the program that tracks contiguous movable areas, so that the solver doesn't have to move the player. The [Sokobano wiki](http://sokobano.de/wiki/index.php?title=Solver#Normalizing_the_player_position) has a great explanation of the idea behind it.
- Check if letting the width, height, and map be #defines helps the compiler with optimization
- ? How to represent the map in such a way that only pushable edges are evaluated?
- Let automated tests check that basic functionality works, with the most fragile feature being contiguous movable areas
- Allow the user to turn on the asan build by passing an optional command argument
- Try using a swap-remove array in `bfs.c` instead of using `strdup()` + `free()`, by storing map strings and paths in a static array
- Try turning `map` into a flattened 2D array, getting the index with `x + y * width`
