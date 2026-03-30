#pragma once

#include <memory>
#include <vector>

class Algorithm;

std::vector<std::unique_ptr<Algorithm>> createAlgorithms();