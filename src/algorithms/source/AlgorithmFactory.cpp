#include "../header/AlgorithmFactory.h"
#include "../header/bfs.h"
#include "../header/dfs.h"
#include "../header/dijkstra.h"
#include "../header/AStar.h"

std::vector<std::unique_ptr<Algorithm>> createAlgorithms() {
    std::vector<std::unique_ptr<Algorithm>> algorithms;

    algorithms.push_back(std::make_unique<BFS>());
    algorithms.push_back(std::make_unique<DFS>());
    algorithms.push_back(std::make_unique<Dijkstra>());
    algorithms.push_back(std::make_unique<AStar>());

    return algorithms;
}