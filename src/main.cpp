#include "maze/Maze.h"
#include "maze/MazeGenerator.h"
#include "ui/RendererSFML.h"

int main() {
    const int width  = 101;
    const int height = 71;
    const int cellSize = 10;

    Maze maze(width, height);
    maze.setStart({1, 1});
    maze.setGoal({width - 2, height - 2});

    MazeGenerator::recursiveBacktracker(maze, 0);

    RendererSFML renderer(maze, cellSize);
    renderer.run();

    return 0;
}