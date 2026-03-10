#include "../header/dijkstra.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>

static inline int keyD(const Coord& c, int w) {
    return c.second * w + c.first;
}

class DijkstraImpl : public Algorithm {
public:
    std::string name() const override { return "Dijkstra"; }

    Result solve(const Maze& maze, StepCallback cb = nullptr) override {
        Result r{};
        auto t0 = std::chrono::high_resolution_clock::now();

        int w = maze.width();
        int h = maze.height();
        int n = w * h;

        const double INF = std::numeric_limits<double>::infinity();
        std::vector<double> dist(n, INF);
        std::vector<Coord> parent(n, {-1, -1});
        std::vector<int> visited(n, 0);

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
        > pq(cmp);

        int ks = keyD(start, w);
        dist[ks] = 0.0;
        pq.push({0.0, start});

        while (!pq.empty()) {
            auto [d, cur] = pq.top();
            pq.pop();
            int kc = keyD(cur, w);
            if (visited[kc]) continue;
            visited[kc] = 1;
            r.nodesExpanded++;

            if (cb) cb(cur);

            if (cur == goal) {
                r.found = true;
                break;
            }

            for (auto nb : maze.neighbors(cur)) {
                int kn = keyD(nb, w);
                double nd = d + 1.0; // all edges weight 1
                if (nd < dist[kn]) {
                    dist[kn] = nd;
                    parent[kn] = cur;
                    pq.push({nd, nb});
                }
            }
        }

        if (r.found) {
            Coord cur = goal;
            while (cur != start) {
                r.path.push_back(cur);
                cur = parent[keyD(cur, w)];
            }
            r.path.push_back(start);
            std::reverse(r.path.begin(), r.path.end());
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        r.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        return r;
    }
};

Algorithm* createDijkstra() {
    return new DijkstraImpl();
}
