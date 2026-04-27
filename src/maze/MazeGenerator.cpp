#include "MazeGenerator.h"

#include <algorithm>
#include <random>
#include <stack>
#include <vector>

#include "../algorithms/header/bfs.h"

// Anonymous namespace keeps this constant private to this file.
namespace {
    // Movement directions for maze carving.
    // Each move jumps two cells so there is always one wall cell between
    // the current cell and the next cell.
    const std::vector<Coord> directions = {
        {2, 0}, {-2, 0}, {0, 2}, {0, -2}
    };
}

// Generates a random maze using the recursive backtracking algorithm.
// The algorithm behaves like depth-first search:
// move to a random unvisited neighbor, carve a path, and backtrack when stuck.
void MazeGenerator::recursiveBacktracker(Maze& maze, unsigned int seed)
{
    // Create random number generator.
    // If seed is nonzero, generation is repeatable.
    // If seed is zero, use a random device for unpredictable mazes.
    std::mt19937 rng(seed ? seed : std::random_device{}());

    const int w = maze.width();
    const int h = maze.height();

    // Start by making every cell a wall.
    // The algorithm will carve open passages from this solid grid.
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            maze.setWall(x, y, true);

    // Stack stores the current path.
    // It allows the algorithm to backtrack when no unvisited neighbors remain.
    std::stack<Coord> st;

    // Start carving from cell (1, 1).
    // Odd coordinates are used as maze cells; even coordinates often remain walls.
    Coord start = {1, 1};
    maze.setWall(start.first, start.second, false);
    st.push(start);

    // Continue until every reachable maze cell has been processed.
    while (!st.empty()) {
        Coord cur = st.top();

        std::vector<Coord> neighbors;

        // Look for unvisited neighbors two cells away.
        for (const auto& d : directions) {
            Coord nb = {cur.first + d.first, cur.second + d.second};

            // A neighbor is valid if it is inside the maze and still a wall.
            // "Still a wall" means it has not been visited/carved yet.
            if (maze.inBounds(nb.first, nb.second) &&
                maze.isWall(nb.first, nb.second))
            {
                neighbors.push_back(nb);
            }
        }

        if (!neighbors.empty()) {
            // Pick one valid neighbor randomly.
            std::uniform_int_distribution<> dist(0, (int)neighbors.size() - 1);
            Coord next = neighbors[dist(rng)];

            // Find the wall cell between current cell and chosen neighbor.
            Coord between = {
                (cur.first + next.first) / 2,
                (cur.second + next.second) / 2
            };

            // Carve the neighbor and the wall between them.
            maze.setWall(next.first, next.second, false);
            maze.setWall(between.first, between.second, false);

            // Continue DFS from the chosen neighbor.
            st.push(next);
        } else {
            // No unvisited neighbors remain, so backtrack.
            st.pop();
        }
    }

    // Force start and goal cells to be open.
    // This prevents them from accidentally staying blocked.
    maze.setWall(maze.getStart().first, maze.getStart().second, false);
    maze.setWall(maze.getGoal().first, maze.getGoal().second, false);

    // Validate that the generated maze has a path from start to goal.
    // If not, regenerate using a different seed.
    if (!isSolvable(maze)) {
        recursiveBacktracker(maze, seed + 1);
    }
}

// Checks whether the maze has a valid path from start to goal.
// BFS is used because it reliably finds a path in an unweighted grid if one exists.
bool MazeGenerator::isSolvable(Maze& maze)
{
    BFS bfs;

    // Run BFS without a visualization callback.
    Result res = bfs.solve(maze, nullptr);

    // found is true only if BFS reached the goal.
    return res.found;
}