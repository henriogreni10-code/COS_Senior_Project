#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

class Maze;

using Coord = std::pair<int, int>;

struct Result {
    bool found = false;
    std::vector<Coord> path;
    int nodesExpanded = 0;
    double timeMs = 0.0;
};

class Algorithm {
public:
    using StepCallback = std::function<void(const Coord&)>;

    virtual ~Algorithm() = default;

    virtual std::string getName() const = 0;
    virtual Result solve(const Maze& maze, StepCallback cb = nullptr) = 0;
};