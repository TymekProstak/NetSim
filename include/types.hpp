#pragma once

#include <functional>

using ElementID = int;
using Time = int;
using TimeOffset = int;

using ProbabilityGenerator = std::function<double()>;

// Single declaration for the whole project (definition in src/helpers.cpp)
extern ProbabilityGenerator probability_generator;

