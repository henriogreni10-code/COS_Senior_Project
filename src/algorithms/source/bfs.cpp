#include "../header/bfs.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <chrono>

static inline int keyBFS(const Coord& c, int w) {
    return c.second * w + c.first;
}

class BFSImpl : public Algorithm {
public:
    std::string name() const override { return "BFS"; }

    Result solve(const Maze& maze, StepCallback cb = nullptr) override {
        Result r{};
        auto t0 = std::chrono::high_resolution_clock::now();
        int w = maze.width();
        int h = maze.height();
        int n = w * h;
        std::vector<int> visited(n, 0);
        std::vector<Coord> parent(n, {-1, -1});
        Coord start = maze.getStart();
        Coord goal  = maze.getGoal();
        std::queue<Coord> q;
        q.push(start);
        visited[keyBFS(start, w)] = 1;
        while (!q.empty()) {
            Coord cur = q.front();
            q.pop();
            r.nodesExpanded++;
            if (cb) cb(cur);
            if (cur == goal) {
                r.found = true;
                break;
            }
            for (auto nb : maze.neighbors(cur)) {
                int k = keyBFS(nb, w);
                if (!visited[k]) {
                    visited[k] = 1;
                    parent[k] = cur;
                    q.push(nb);
                }
            }
        }
        if (r.found) {
            Coord cur = goal;
            while (cur != start) {
                r.path.push_back(cur);
                cur = parent[keyBFS(cur, w)];
            }
            r.path.push_back(start);
            std::reverse(r.path.begin(), r.path.end());
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        r.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        return r;
    }
};

Algorithm* createBFS() {
    return new BFSImpl();
}
