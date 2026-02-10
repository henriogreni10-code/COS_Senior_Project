#include "../header/AStar.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include <cmath>
#include <iostream>

static inline int keyA(const Coord& c, int w) {
    return c.second * w + c.first;
}

static inline double heuristic(const Coord& a, const Coord& b) {
    // Manhattan distance
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

class AStarImpl : public Algorithm {
public:
    std::string name() const override { return "A*"; }

    Result solve(const Maze& maze, StepCallback cb = nullptr) override {
        Result r{};
        auto t0 = std::chrono::high_resolution_clock::now();

        int w = maze.width();
        int h = maze.height();
        int n = w * h;

        const double INF = std::numeric_limits<double>::infinity();
        std::vector<double> gScore(n, INF);
        std::vector<double> fScore(n, INF);
        std::vector<Coord> parent(n, {-1, -1});
        std::vector<int> inOpen(n, 0);

        Coord start = maze.getStart();
        Coord goal  = maze.getGoal();

        auto cmp = [](const std::pair<double, Coord>& a,
                      const std::pair<double, Coord>& b) {
            return a.first > b.first;
        };
        std::priority_queue<
            std::pair<double, Coord>,
            std::vector<std::pair<double, Coord>>,
            decltype(cmp)
        > open(cmp);

        int ks = keyA(start, w);
        gScore[ks] = 0.0;
        fScore[ks] = heuristic(start, goal);
        open.push({fScore[ks], start});
        inOpen[ks] = 1;

        while (!open.empty()) {
            auto [f, cur] = open.top();
            open.pop();

            int kc = keyA(cur, w);
            if (!inOpen[kc]) continue;
            inOpen[kc] = 0;
            r.nodesExpanded++;

            if (cb) cb(cur);

            if (cur == goal) {
                r.found = true;
                break;
            }

            for (auto nb : maze.neighbors(cur)) {
                int kn = keyA(nb, w);
                double tentative = gScore[kc] + 1.0;

                if (tentative < gScore[kn]) {
                    parent[kn] = cur;
                    gScore[kn] = tentative;
                    fScore[kn] = tentative + heuristic(nb, goal);
                    open.push({fScore[kn], nb});
                    inOpen[kn] = 1;
                }
            }
        }

        if (r.found) {
            Coord cur = goal;
            while (cur != start) {
                r.path.push_back(cur);
                cur = parent[keyA(cur, w)];
            }
            r.path.push_back(start);
            std::reverse(r.path.begin(), r.path.end());
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        r.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        return r;
    }
};

Algorithm* createAStar() {
    return new AStarImpl();
}
