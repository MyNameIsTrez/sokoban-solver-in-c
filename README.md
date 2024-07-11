# Sokoban solver in C

[Sokoban](https://en.wikipedia.org/wiki/Sokoban) is a game in which the player pushes boxes around in a warehouse, trying to get them to storage locations.

![GIF of a player pushing boxes around in Sokoban](sokoban.gif)

Writing a solver for Sokoban is similar to writing a chess engine. Just like with chess, the branching factor (the number of possible moves on each turn) of a complex Sokoban level can be ridiculously huge. The number of chess and Sokoban boards is `O(b^d)`, where `b` is the branching factor, and `d` is the depth of the shallowest solution. In other words, they both exhibit exponential growth.

The [Sokobano wiki's Solver page](http://sokobano.de/wiki/index.php?title=Solver) contains lots of juicy tips on how to approach writing a solver.

`bfs.c` ([breadth-first search](https://en.wikipedia.org/wiki/Breadth-first_search)) is best when the branching factor is near 1. The downside is that it runs out of memory in big maps. It is guaranteed to find the shortest path.

`iddfs.c` ([iterative deepening depth-first search](https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search)) is best when the branching factor is quite a bit higher than 1. It doesn't run out of memory in big maps. It is guaranteed to find the shortest path.

`area.c` is based on `iddfs.c`, but the major difference is that it doesn't make the player walk around one step at a time. Instead, it tracks which floor tiles are reachable by the player, so that it knows which boxes the player is able to push, if the player were to walk up to them. This way, the solver can just push reachable boxes directly. The implementation floodfills every time a box is pushed, where any time a new box is now exposed, it will recursively also get pushed. It IS NOT guaranteed to find the shortest path.

## Visualizig reachable areas

The player is only able to move to the right here:

```
######
#.$ ##
##@$.#
######
```

After which the player's reachable area grows from 1 to 3 tiles:

```
######
#.$ ##
## @*#
######
```

The player is then able to push the box on the top to the left into the remaining storage location. The solution ends up with 4 reachable tiles:

```
######
#*@ ##
##  *#
######
```

Another important detail to keep in mind that this:

```
#####
#@$.#
#####
```

Is a completely different setup from this:

```
#####
# $+#
#####
```

So this is why it is important for the hashing to take which area the player is standing in into account. This is done by getting the top-left reachable position of the player, and letting `stringify_map()` append the position to the hashed string.

## Running

`./tests.sh`

## Visualizing solutions

[Henry Kautz](https://henrykautz.com/sokoban/Sokoban.html) has a great website for visualizing Sokoban maps and solutions. The [help](https://henrykautz.com/sokoban/help.html) button at the bottom of that page explains the file format his website expects.

## Future plans

- Check if letting the width, height, and map be #defines helps the compiler with optimization
- Let automated tests check that basic functionality works, with the most fragile feature being contiguous movable areas
- Allow the user to turn on the asan build by passing an optional command argument
- Try using a swap-remove array in `bfs.c` instead of using `strdup()` + `free()`, by storing map strings and paths in a static array
- Try turning `map` into a flattened 2D array, getting the index with `x + y * width`
- Profile whether turning `area.c` its `pushable` array from a local one into a global one, by having its values be `struct push { enum push_direction; size_t x; size_t y; };`. Every solve() call has `size_t starting_pushable_length = pushable_length;`
- Profile whether using `:char` is faster for the enum than the default type of `:int` (note that this requires compiling with `-std=c2x`)
- Replace the `y--;` -> `solve(x, y);` -> `y++;` in `up()`, `down()`, `left()` and `right()` with `solve(x, y-1);`, in `iddfs.c` and `bfs.c`
