#include "MazeGenerator.h"
#include "../algorithms/header/bfs.h"
#include <random>
#include <iostream>

extern Algorithm* createBFS();

static std::vector<Coord> shuffleDirs(std::mt19937& rng)
{
    std::vector<Coord> dirs = {
        { 2, 0 }, { -2, 0 }, { 0, 2 }, { 0, -2 }
    };
    std::shuffle(dirs.begin(), dirs.end(), rng);
    return dirs;
}

void MazeGenerator::recursiveBacktracker(Maze& maze, unsigned seed)
{
    int w = maze.width();
    int h = maze.height();

    maze.clear();

    std::mt19937 rng(seed ? seed : std::random_device{}());

    Coord start = maze.getStart();
    if (start.first % 2 == 0)  start.first--;
    if (start.second % 2 == 0) start.second--;

    Coord goal = maze.getGoal();
    if (goal.first % 2 == 0)  goal.first--;
    if (goal.second % 2 == 0) goal.second--;

    maze.setStart(start);
    maze.setGoal(goal);

    maze.setWall(start.first, start.second, false);

    std::vector<Coord> stack;
    stack.push_back(start);

    while (!stack.empty())
    {
        Coord cur = stack.back();
        bool carved = false;

        auto dirs = shuffleDirs(rng);
        for (auto d : dirs)
        {
            int nx = cur.first + d.first;
            int ny = cur.second + d.second;

            if (!maze.inBounds(nx, ny))
                continue;

            if (maze.isWall(nx, ny))
            {
                int mx = cur.first + d.first / 2;
                int my = cur.second + d.second / 2;

                maze.setWall(mx, my, false);
                maze.setWall(nx, ny, false);

                stack.push_back({nx, ny});
                carved = true;
                break;
            }
        }

        if (!carved)
            stack.pop_back();
    }

    Algorithm* bfs = createBFS();
    Result result = bfs->solve(maze);
    delete bfs;

    if (!result.found)
    {
        std::cerr << "⚠ Maze unsolvable — regenerating...\n";
        recursiveBacktracker(maze, seed + 1);
        return;
    }

}
