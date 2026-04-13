#include "../header/BellmanFord.h"
#include "../../maze/Maze.h"

#include <algorithm>
#include <chrono>
#include <limits>
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

        if (goal != start && parent[toIndex(goal, w)] == Coord{-1, -1}) {
            return r;
        }

        r.found = true;

        Coord cur = goal;
        while (cur != start) {
            r.path.push_back(cur);
            cur = parent[toIndex(cur, w)];
        }

        r.path.push_back(start);
        std::reverse(r.path.begin(), r.path.end());
        return r;
    }
}

std::string BellmanFord::getName() const {
    return "Bellman-Ford";
}

Result BellmanFord::solve(const Maze& maze, StepCallback cb) {
    auto t0 = std::chrono::high_resolution_clock::now();

    const int w = maze.width();
    const int h = maze.height();
    const int n = w * h;

    const Coord start = maze.getStart();
    const Coord goal = maze.getGoal();

    const double INF = std::numeric_limits<double>::infinity();

    std::vector<double> dist(n, INF);
    std::vector<Coord> parent(n, {-1, -1});
    std::vector<bool> reachable(n, false);

    dist[toIndex(start, w)] = 0.0;
    reachable[toIndex(start, w)] = true;

    int expanded = 0;

    for (int pass = 0; pass < n - 1; ++pass) {
        bool changed = false;

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (maze.isWall(x, y)) {
                    continue;
                }

                Coord cur{x, y};
                int kc = toIndex(cur, w);

                if (dist[kc] == INF) {
                    continue;
                }

                expanded++;

                if (cb) {
                    cb(cur);
                }

                for (const Coord& nb : maze.neighbors(cur)) {
                    int kn = toIndex(nb, w);
                    double nd = dist[kc] + 1.0;

                    if (nd < dist[kn]) {
                        dist[kn] = nd;
                        parent[kn] = cur;
                        reachable[kn] = true;
                        changed = true;
                    }
                }
            }
        }

        if (!changed) {
            break;
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!reachable[toIndex(goal, w)]) {
        Result r;
        r.nodesExpanded = expanded;
        r.timeMs = timeMs;
        return r;
    }

    return buildResult(parent, start, goal, w, expanded, timeMs);
}