# Sokoban solver in C

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
