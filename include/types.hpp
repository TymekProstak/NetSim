#pragma once

#include <functional>

using ElementID = int;
using Time = int;
using TimeOffset = int;

// Unified spelling used across the project.
using ProbabilityGenerator = std::function<double()>;
ProbabilityGenerator probability_generator = []() {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
};

