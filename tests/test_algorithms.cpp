#include "test_utils.h"
#include "../src/maze/Maze.h"
#include "../src/algorithm.h"
#include <memory>
#include <vector>
#include <cstdlib>
#include <cmath>

// Factory functions implemented in the cpp files
Algorithm* createBFS();
Algorithm* createDFS();
Algorithm* createDijkstra();
Algorithm* createAStar();

static Maze makeOpenMaze(int w, int h) {
    Maze maze(w, h);
    for (int y = 0; y < maze.height(); ++y) {
        for (int x = 0; x < maze.width(); ++x) {
            maze.setWall(x, y, false);
        }
    }
    return maze;
}

static Maze makeSinglePathMaze() {
    Maze maze(7, 7);
    maze.clear();

    // Carve only one corridor:
    // (1,1)->(2,1)->(3,1)->(3,2)->(3,3)->(4,3)->(5,3)->(5,4)->(5,5)
    std::vector<Coord> cells = {
        {1,1},{2,1},{3,1},{3,2},{3,3},{4,3},{5,3},{5,4},{5,5}
    };

    for (const auto& c : cells) {
        maze.setWall(c.first, c.second, false);
    }

    maze.setStart({1,1});
    maze.setGoal({5,5});
    return maze;
}

static void assert_valid_path(const Maze& maze, const Result& r) {
    ASSERT_TRUE(r.found);
    ASSERT_FALSE(r.path.empty());
    ASSERT_EQ(r.path.front(), maze.getStart());
    ASSERT_EQ(r.path.back(), maze.getGoal());

    for (size_t i = 1; i < r.path.size(); ++i) {
        const auto& a = r.path[i - 1];
        const auto& b = r.path[i];
        int manhattan = std::abs(a.first - b.first) + std::abs(a.second - b.second);
        ASSERT_EQ(manhattan, 1);
        ASSERT_FALSE(maze.isWall(b.first, b.second));
    }
}

static void test_algorithm_on_simple_path(Algorithm* algo) {
    std::unique_ptr<Algorithm> ptr(algo);
    Maze maze = makeSinglePathMaze();

    std::vector<Coord> visited;
    Result r = ptr->solve(maze, [&](const Coord& c) {
        visited.push_back(c);
    });

    assert_valid_path(maze, r);
    ASSERT_EQ(r.path.size(), 9u);
    ASSERT_GT(r.nodesExpanded, 0u);
    ASSERT_GE(r.timeMs, 0.0);
    ASSERT_FALSE(visited.empty());
}

static void test_algorithm_start_equals_goal(Algorithm* algo) {
    std::unique_ptr<Algorithm> ptr(algo);
    Maze maze = makeOpenMaze(5, 5);
    maze.setStart({1,1});
    maze.setGoal({1,1});

    Result r = ptr->solve(maze);
    ASSERT_TRUE(r.found);
    ASSERT_EQ(r.path.size(), 1u);
    ASSERT_EQ(r.path.front(), Coord(1,1));
}

static void test_algorithm_unsolvable(Algorithm* algo) {
    std::unique_ptr<Algorithm> ptr(algo);
    Maze maze(7, 7);
    maze.clear();

    // Only start and goal are open, no connection.
    maze.setWall(1, 1, false);
    maze.setWall(5, 5, false);
    maze.setStart({1,1});
    maze.setGoal({5,5});

    Result r = ptr->solve(maze);
    ASSERT_FALSE(r.found);
    ASSERT_TRUE(r.path.empty());
    ASSERT_GT(r.nodesExpanded, 0u);
}

static void test_shortest_path_algorithms() {
    Maze maze = makeOpenMaze(7, 7);
    maze.setStart({1,1});
    maze.setGoal({5,5});

    std::unique_ptr<Algorithm> bfs(createBFS());
    std::unique_ptr<Algorithm> dij(createDijkstra());
    std::unique_ptr<Algorithm> astar(createAStar());

    Result rb = bfs->solve(maze);
    Result rd = dij->solve(maze);
    Result ra = astar->solve(maze);

    assert_valid_path(maze, rb);
    assert_valid_path(maze, rd);
    assert_valid_path(maze, ra);

    // Manhattan distance from (1,1) to (5,5) is 8 edges => 9 nodes in path
    ASSERT_EQ(rb.path.size(), 9u);
    ASSERT_EQ(rd.path.size(), 9u);
    ASSERT_EQ(ra.path.size(), 9u);
}

void run_algorithm_tests() {
    test_algorithm_on_simple_path(createBFS());
    test_algorithm_on_simple_path(createDFS());
    test_algorithm_on_simple_path(createDijkstra());
    test_algorithm_on_simple_path(createAStar());

    test_algorithm_start_equals_goal(createBFS());
    test_algorithm_start_equals_goal(createDFS());
    test_algorithm_start_equals_goal(createDijkstra());
    test_algorithm_start_equals_goal(createAStar());

    test_algorithm_unsolvable(createBFS());
    test_algorithm_unsolvable(createDFS());
    test_algorithm_unsolvable(createDijkstra());
    test_algorithm_unsolvable(createAStar());

    test_shortest_path_algorithms();
}