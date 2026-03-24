#include "test_utils.h"
#include "../src/maze/Maze.h"
#include "../src/maze/MazeGenerator.h"
#include "../src/algorithm.h"
#include <memory>
#include <vector>

Algorithm* createBFS();

// Maze Generation test
void run_generator_tests() {
    {
        Maze maze(21, 21);
        maze.setStart({1,1});
        maze.setGoal({19,19});

        MazeGenerator::recursiveBacktracker(maze, 12345);

        ASSERT_FALSE(maze.isWall(maze.getStart().first, maze.getStart().second));
        ASSERT_FALSE(maze.isWall(maze.getGoal().first, maze.getGoal().second));

        std::unique_ptr<Algorithm> bfs(createBFS());
        Result r = bfs->solve(maze);

        ASSERT_TRUE(r.found);
        ASSERT_FALSE(r.path.empty());
        ASSERT_EQ(r.path.front(), maze.getStart());
        ASSERT_EQ(r.path.back(), maze.getGoal());
    }

    {
        Maze maze1(21, 21);
        Maze maze2(21, 21);

        maze1.setStart({1,1});
        maze1.setGoal({19,19});
        maze2.setStart({1,1});
        maze2.setGoal({19,19});

        MazeGenerator::recursiveBacktracker(maze1, 777);
        MazeGenerator::recursiveBacktracker(maze2, 777);

        // Same seed should produce same wall layout
        for (int y = 0; y < maze1.height(); ++y) {
            for (int x = 0; x < maze1.width(); ++x) {
                ASSERT_EQ(maze1.isWall(x, y), maze2.isWall(x, y));
            }
        }
    }

    {
        Maze maze(20, 20); // constructor normalizes to odd
        maze.setStart({2,2});
        maze.setGoal({18,18});

        MazeGenerator::recursiveBacktracker(maze, 42);

        // Generator normalizes start/goal to odd coordinates
        ASSERT_TRUE(maze.getStart().first % 2 == 1);
        ASSERT_TRUE(maze.getStart().second % 2 == 1);
        ASSERT_TRUE(maze.getGoal().first % 2 == 1);
        ASSERT_TRUE(maze.getGoal().second % 2 == 1);
    }
}