#include "helpers.hpp"
#include "types.hpp"

#include <random>
#include <functional>

std::random_device rd{};
std::mt19937 rng{rd()};

ProbabilityGenerator probability_generator = []() {
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
};