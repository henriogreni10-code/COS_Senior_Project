#pragma once

#include "Maze.h"

class MazeGenerator {
public:
    static void recursiveBacktracker(Maze& maze, unsigned int seed = 0);

private:
    static bool isSolvable(Maze& maze);
};