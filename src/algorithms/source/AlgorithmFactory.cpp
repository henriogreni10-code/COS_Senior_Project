#include "../header/AlgorithmFactory.h"

#include "../header/AStar.h"
#include "../header/bfs.h"
#include "../header/dfs.h"
#include "../header/dijkstra.h"

std::vector<std::unique_ptr<Algorithm>> createAlgorithms() {
    std::vector<std::unique_ptr<Algorithm>> v;

    v.push_back(std::make_unique<DFS>());
    v.push_back(std::make_unique<BFS>());
    v.push_back(std::make_unique<Dijkstra>());
    v.push_back(std::make_unique<AStar>());

    return v;
}