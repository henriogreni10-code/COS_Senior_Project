#include "../header/AStar.h"
#include "../../maze/Maze.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

namespace {
    int toIndex(const Coord& c, int w) {
        return c.second * w + c.first;
    }

    double heuristic(const Coord& a, const Coord& b) {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    }

    Result buildResult(const std::vector<Coord>& parent,
                       Coord start, Coord goal, int w,
                       int nodes, double timeMs)
    {
        Result r;
        r.nodesExpanded = nodes;
        r.timeMs = timeMs;

        if (goal != start && parent[toIndex(goal,w)] == Coord{-1,-1})
            return r;

        r.found = true;

        Coord cur = goal;
        while (cur != start) {
            r.path.push_back(cur);
            cur = parent[toIndex(cur,w)];
        }
        r.path.push_back(start);
        std::reverse(r.path.begin(), r.path.end());
        return r;
    }
}

std::string AStar::getName() const {
    return "A*";
}

Result AStar::solve(const Maze& maze, StepCallback cb) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int w = maze.width();
    int h = maze.height();
    int n = w * h;

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> gScore(n, INF);
    std::vector<double> fScore(n, INF);
    std::vector<Coord> parent(n, {-1,-1});
    std::vector<bool> closed(n, false);

    Coord start = maze.getStart();
    Coord goal  = maze.getGoal();

    using Node = std::pair<double, Coord>;
    auto cmp = [](const Node& a, const Node& b) {
        return a.first > b.first;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

    int ks = toIndex(start,w);
    gScore[ks] = 0.0;
    fScore[ks] = heuristic(start,goal);
    open.push({fScore[ks], start});

    int expanded = 0;

    while (!open.empty()) {
        Coord cur = open.top().second;
        open.pop();

        int kc = toIndex(cur,w);
        if (closed[kc]) continue;
        closed[kc] = true;

        expanded++;

        if (cb) cb(cur);

        if (cur == goal) break;

        for (auto nb : maze.neighbors(cur)) {
            int kn = toIndex(nb,w);
            if (closed[kn]) continue;

            double tentative = gScore[kc] + 1.0;

            if (tentative < gScore[kn]) {
                parent[kn] = cur;
                gScore[kn] = tentative;
                fScore[kn] = tentative + heuristic(nb,goal);
                open.push({fScore[kn], nb});
            }
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double,std::milli>(t1-t0).count();

    return buildResult(parent,start,goal,w,expanded,timeMs);
}