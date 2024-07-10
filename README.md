# Sokoban solver in C

[Sokoban](https://en.wikipedia.org/wiki/Sokoban) is a game in which the player pushes boxes around in a warehouse, trying to get them to storage locations.

![GIF of a player pushing boxes around in Sokoban](sokoban.gif)

Writing a solver for Sokoban is similar to writing a chess engine. Just like with chess, the branching factor (the number of possible moves on each turn) of a complex Sokoban level can be ridiculously huge. The number of chess and Sokoban boards is `O(b^d)`, where `b` is the branching factor, and `d` is the depth of the shallowest solution. In other words, they both exhibit exponential growth.

`bfs.c` ([breadth-first search](https://en.wikipedia.org/wiki/Breadth-first_search)) is best when the branching factor is near 1. The downside is that it runs out of memory in big maps.

`iddfs.c` ([iterative deepening depth-first search](https://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search)) is best when the branching factor is quite a bit higher than 1. It doesn't run out of memory in big maps.

`area.c` is based on `iddfs.c`, but the major difference is that it doesn't make the player walk around one step at a time. Instead, it tracks which floor tiles are reachable by the player, so that it knows which boxes the player is able to push, if the player were to walk up to them. This way, the solver can just push reachable boxes directly. The implementation floodfills every time a box is pushed, where any time a new box is now exposed, it will recursively also get pushed.

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
