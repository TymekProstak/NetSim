#pragma once

#include <functional>

using ElementID = int;
using Time = int;
using TimeOffset = int;

using PropabilityGenerator = std::function<double()>;
