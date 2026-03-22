#pragma once
#include <bitset>

#include "engine/ecs/componentArray.hpp"

// A signature is a bitset where each bit represents whether an entity has a specific component type or not.
namespace ecs
{
    // The maximum number of components is defined by MAX_COMPONENTS, which is set to the maximum number of entities for simplicity.
    using Signature = std::bitset<MAX_COMPONENTS>;
}
