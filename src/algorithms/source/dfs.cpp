#include "../header/dfs.h"
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

std::string DFS::getName() const {
    return "A*";
}

Algorithm::Path DFS::solve(Maze& maze, VisitCallback onVisit) {
    const int width = maze.width();
    const int height = maze.height();
    const int totalCells = width * height;

    const Coord start = maze.getStart();
    const Coord goal = maze.getGoal();

    std::vector<bool> visited(totalCells, false);
    std::vector<Coord> parent(totalCells, {-1, -1});
    std::queue<Coord> q;

    q.push(start);
    visited[toIndex(start, width)] = true;

    while (!q.empty()) {
        Coord current = q.front();
        q.pop();

        if (onVisit) {
            onVisit(current.first, current.second);
        }

        if (current == goal) {
            return reconstructPath(parent, start, goal, width);
        }

        for (const Coord& neighbor : maze.neighbors(current)) {
            int index = toIndex(neighbor, width);
            if (!visited[index]) {
                visited[index] = true;
                parent[index] = current;
                q.push(neighbor);
            }
        }
    }

    return {};
}