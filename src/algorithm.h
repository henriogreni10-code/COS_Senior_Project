#pragma once

#include "./maze/Maze.h"
#include <vector>
#include <string>
#include <functional>

class Maze;

class Algorithm {
public:
    using Coord = std::pair<int, int>;
    using Path = std::vector<Coord>;
    using VisitCallback = std::function<void(int, int)>;

    virtual ~Algorithm() = default;
    virtual std::string getName() const = 0;
    virtual Path solve(Maze& maze, VisitCallback onVisit = nullptr) = 0;
};
