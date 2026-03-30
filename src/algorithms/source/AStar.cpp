#include "../header/AStar.h"
#include "../../maze/Maze.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

namespace {
    int toIndex(const Algorithm::Coord& cell, int width) {
        return cell.second * width + cell.first;
    }

    double heuristic(const Algorithm::Coord& a, const Algorithm::Coord& b) {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    }

    Algorithm::Path reconstructPath(
        const std::vector<Algorithm::Coord>& parent,
        Algorithm::Coord start,
        Algorithm::Coord goal,
        int width
    ) {
        Algorithm::Path path;

        if (goal != start && parent[toIndex(goal, width)] == Algorithm::Coord{-1, -1}) {
            return path;
        }

        Algorithm::Coord current = goal;
        while (current != start) {
            path.push_back(current);
            current = parent[toIndex(current, width)];
        }

        path.push_back(start);
        std::reverse(path.begin(), path.end());
        return path;
    }
}

std::string AStar::getName() const {
    return "A*";
}

Algorithm::Path AStar::solve(Maze& maze, VisitCallback onVisit) {
    const int width = maze.width();
    const int height = maze.height();
    const int totalCells = width * height;

    const Coord start = maze.getStart();
    const Coord goal = maze.getGoal();

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> gScore(totalCells, INF);
    std::vector<double> fScore(totalCells, INF);
    std::vector<Coord> parent(totalCells, {-1, -1});
    std::vector<bool> closed(totalCells, false);

    using Node = std::pair<double, Coord>;

    auto compare = [](const Node& a, const Node& b) {
        return a.first > b.first;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(compare)> openSet(compare);

    const int startIndex = toIndex(start, width);
    gScore[startIndex] = 0.0;
    fScore[startIndex] = heuristic(start, goal);
    openSet.push({fScore[startIndex], start});

    while (!openSet.empty()) {
        Coord current = openSet.top().second;
        openSet.pop();
        const int currentIndex = toIndex(current, width);
        if (closed[currentIndex]) {
            continue;
        }
        closed[currentIndex] = true;
        if (onVisit) {
            onVisit(current.first, current.second);
        }
        if (current == goal) {
            return reconstructPath(parent, start, goal, width);
        }
        for (const Coord& neighbor : maze.neighbors(current)) {
            const int neighborIndex = toIndex(neighbor, width);
            if (closed[neighborIndex]) {
                continue;
            }
            const double tentativeGScore = gScore[currentIndex] + 1.0;
            if (tentativeGScore < gScore[neighborIndex]) {
                parent[neighborIndex] = current;
                gScore[neighborIndex] = tentativeGScore;
                fScore[neighborIndex] = tentativeGScore + heuristic(neighbor, goal);
                openSet.push({fScore[neighborIndex], neighbor});
            }
        }
    }
    return {};
}