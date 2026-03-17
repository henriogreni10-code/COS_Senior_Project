#include "test_utils.h"
#include "../src/maze/Maze.h"
#include <vector>
#include <algorithm>

static Maze makeOpenMaze(int w, int h) {
    Maze maze(w, h);
    for (int y = 0; y < maze.height(); ++y) {
        for (int x = 0; x < maze.width(); ++x) {
            maze.setWall(x, y, false);
        }
    }
    return maze;
}

void run_maze_tests() {
    {
        Maze maze(10, 8);
        // Constructor forces odd dimensions
        ASSERT_EQ(maze.width(), 9);
        ASSERT_EQ(maze.height(), 7);
    }

    {
        Maze maze(7, 7);
        ASSERT_TRUE(maze.inBounds(0, 0));
        ASSERT_TRUE(maze.inBounds(6, 6));
        ASSERT_FALSE(maze.inBounds(-1, 0));
        ASSERT_FALSE(maze.inBounds(0, -1));
        ASSERT_FALSE(maze.inBounds(7, 0));
        ASSERT_FALSE(maze.inBounds(0, 7));
    }

    {
        Maze maze(7, 7);
        // Initially all walls
        ASSERT_TRUE(maze.isWall(1, 1));
        maze.setWall(1, 1, false);
        ASSERT_FALSE(maze.isWall(1, 1));
        maze.setWall(1, 1, true);
        ASSERT_TRUE(maze.isWall(1, 1));
    }

    {
        Maze maze = makeOpenMaze(7, 7);
        maze.setStart({1, 1});
        maze.setGoal({5, 5});
        ASSERT_EQ(maze.getStart(), Coord(1, 1));
        ASSERT_EQ(maze.getGoal(), Coord(5, 5));
    }

    {
        Maze maze = makeOpenMaze(7, 7);
        auto n = maze.neighbors({3, 3});
        ASSERT_EQ(n.size(), 4u);
        ASSERT_TRUE(std::find(n.begin(), n.end(), Coord(4, 3)) != n.end());
        ASSERT_TRUE(std::find(n.begin(), n.end(), Coord(2, 3)) != n.end());
        ASSERT_TRUE(std::find(n.begin(), n.end(), Coord(3, 4)) != n.end());
        ASSERT_TRUE(std::find(n.begin(), n.end(), Coord(3, 2)) != n.end());
    }

    {
        Maze maze(7, 7);
        maze.clear();
        for (int y = 0; y < maze.height(); ++y) {
            for (int x = 0; x < maze.width(); ++x) {
                ASSERT_TRUE(maze.isWall(x, y));
            }
        }
    }
}