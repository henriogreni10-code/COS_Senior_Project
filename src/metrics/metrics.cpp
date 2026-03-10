#include "metrics.h"
#include <iostream>

    void Metrics::printSummary(const std::string& algName, const Result& r) {
    std::cout << "=== " << algName << " ===\n";
    std::cout << "Found: " << (r.found ? "YES" : "NO") << "\n";
    std::cout << "Path length: " << r.path.size() << "\n";
    std::cout << "Nodes expanded: " << r.nodesExpanded << "\n";
    std::cout << "Time (ms): " << r.timeMs << "\n";
    std::cout << "====================\n";
}
