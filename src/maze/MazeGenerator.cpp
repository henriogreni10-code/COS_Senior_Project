#include "MazeGenerator.h"

#include <algorithm>
#include <random>
#include <stack>
#include <vector>

#include "../algorithms/header/bfs.h"

namespace {
    const std::vector<Coord> directions = {
        {2, 0}, {-2, 0}, {0, 2}, {0, -2}
    };
}

void MazeGenerator::recursiveBacktracker(Maze& maze, unsigned int seed)
{
    std::mt19937 rng(seed ? seed : std::random_device{}());

    const int w = maze.width();
    const int h = maze.height();

    // Fill everything with walls
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            maze.setWall(x, y, true);

    std::stack<Coord> st;

    Coord start = {1, 1};
    maze.setWall(start.first, start.second, false);
    st.push(start);

    while (!st.empty()) {
        Coord cur = st.top();

        std::vector<Coord> neighbors;

        for (const auto& d : directions) {
            Coord nb = {cur.first + d.first, cur.second + d.second};

            if (maze.inBounds(nb.first, nb.second) &&
                maze.isWall(nb.first, nb.second))
            {
                neighbors.push_back(nb);
            }
        }

        if (!neighbors.empty()) {
            std::uniform_int_distribution<> dist(0, (int)neighbors.size() - 1);
            Coord next = neighbors[dist(rng)];

            // carve wall between
            Coord between = {
                (cur.first + next.first) / 2,
                (cur.second + next.second) / 2
            };

            maze.setWall(next.first, next.second, false);
            maze.setWall(between.first, between.second, false);

            st.push(next);
        } else {
            st.pop();
        }
    }

    // Ensure start/goal are open
    maze.setWall(maze.getStart().first, maze.getStart().second, false);
    maze.setWall(maze.getGoal().first, maze.getGoal().second, false);

    // Optional: regenerate until solvable
    if (!isSolvable(maze)) {
        recursiveBacktracker(maze, seed + 1);
    }
}

bool MazeGenerator::isSolvable(Maze& maze)
{
    BFS bfs;
    Result res = bfs.solve(maze, nullptr);
    return res.found;
}