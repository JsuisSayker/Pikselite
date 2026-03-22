#pragma once
#include <bitset>

#include "engine/ecs/componentArray.hpp"

namespace ecs
{
    using Signature = std::bitset<MAX_COMPONENTS>;
}
