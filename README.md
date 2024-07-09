# Sokoban solver in C

## Running

`./tests.sh`

## Future plans

- Memoize by hashing the moves string
- IDDFS
- Make version of the program that tracks contiguous movable areas, so that the solver doesn't have to move the player
- Check if letting the width, height, and map be #defines helps the compiler with optimization
- ? How to represent the map in such a way that only pushable edges are evaluated?
- Let automated tests check that basic functionality works, with the most fragile feature being contiguous movable areas
