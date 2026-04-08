#pragma once
#include "../../algorithm.h"

class DFS : public Algorithm {
public:
    std::string getName() const override;
    Result solve(const Maze& maze, StepCallback cb = nullptr) override;
};