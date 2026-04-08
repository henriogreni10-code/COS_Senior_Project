#include "../header/dfs.h"
#include "../../maze/Maze.h"

#include <algorithm>
#include <chrono>
#include <stack>
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

std::string DFS::getName() const {
    return "DFS";
}

Result DFS::solve(const Maze& maze, StepCallback cb) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int w = maze.width();
    int h = maze.height();
    int n = w * h;

    std::vector<bool> visited(n, false);
    std::vector<Coord> parent(n, {-1,-1});

    Coord start = maze.getStart();
    Coord goal  = maze.getGoal();

    std::stack<Coord> st;

    st.push(start);

    int expanded = 0;

    while (!st.empty()) {
        Coord cur = st.top();
        st.pop();

        int kc = toIndex(cur,w);
        if (visited[kc]) continue;

        visited[kc] = true;
        expanded++;

        if (cb) cb(cur);

        if (cur == goal) break;

        for (auto nb : maze.neighbors(cur)) {
            int kn = toIndex(nb,w);

            if (!visited[kn]) {
                parent[kn] = cur;
                st.push(nb);
            }
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double,std::milli>(t1-t0).count();

    return buildResult(parent,start,goal,w,expanded,timeMs);
}