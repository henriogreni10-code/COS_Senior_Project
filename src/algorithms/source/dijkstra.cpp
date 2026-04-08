#include "../header/dijkstra.h"
#include "../../maze/Maze.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <queue>
#include <vector>

namespace {
    int toIndex(const Coord& c, int w) {
        return c.second * w + c.first;
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

std::string Dijkstra::getName() const {
    return "Dijkstra";
}

Result Dijkstra::solve(const Maze& maze, StepCallback cb) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int w = maze.width();
    int h = maze.height();
    int n = w * h;

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    std::vector<Coord> parent(n, {-1,-1});
    std::vector<bool> closed(n, false);

    Coord start = maze.getStart();
    Coord goal  = maze.getGoal();

    using Node = std::pair<double, Coord>;

    auto cmp = [](const Node& a, const Node& b) {
        return a.first > b.first;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);

    int ks = toIndex(start,w);
    dist[ks] = 0.0;
    pq.push({0.0, start});

    int expanded = 0;

    while (!pq.empty()) {
        Coord cur = pq.top().second;
        pq.pop();

        int kc = toIndex(cur,w);
        if (closed[kc]) continue;

        closed[kc] = true;
        expanded++;

        if (cb) cb(cur);

        if (cur == goal) break;

        for (auto nb : maze.neighbors(cur)) {
            int kn = toIndex(nb,w);
            if (closed[kn]) continue;

            double nd = dist[kc] + 1.0;

            if (nd < dist[kn]) {
                dist[kn] = nd;
                parent[kn] = cur;
                pq.push({nd, nb});
            }
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double,std::milli>(t1-t0).count();

    return buildResult(parent,start,goal,w,expanded,timeMs);
}