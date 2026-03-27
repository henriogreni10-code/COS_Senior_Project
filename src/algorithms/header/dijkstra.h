#pragma once
#include "../../algorithm.h"

class Dijkstra : public Algorithm {
public:
    std::string getName() const override;
    Path solve(Maze& maze, VisitCallback onVisit = nullptr) override;
};
