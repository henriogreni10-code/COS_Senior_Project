#pragma once
#include "./Maze.h"

class MazeGenerator {
public:
    static void recursiveBacktracker(Maze& maze, unsigned seed = 0);
};
