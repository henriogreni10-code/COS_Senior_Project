#pragma once

#include "./maze/Maze.h"
#include <vector>
#include <string>
#include <functional>

struct Result {
    bool found = false;
    std::vector<Coord> path;
    std::size_t nodesExpanded = 0;
    double timeMs = 0.0;
};

class Algorithm {
public:
    using StepCallback = std::function<void(const Coord&)>;
    virtual ~Algorithm() = default;
    virtual Result solve(const Maze& maze, StepCallback cb = nullptr) = 0;
    virtual std::string name() const = 0;
};
