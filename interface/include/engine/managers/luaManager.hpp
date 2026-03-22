#pragma once

#include <string>
#include <iostream>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace engine
{
    class LuaManager
    {
    public:
        LuaManager();
        ~LuaManager() = default;

        /// Load and execute a .lua file. Functions defined in it become callable.
        bool loadScript(const std::string &filepath);

        /// Call a Lua function by name with arbitrary args (forwarded via sol2).
        template <typename... Args>
        sol::protected_function_result call(const std::string &functionName, Args &&...args)
        {
            sol::protected_function fn = _lua[functionName];
            if (!fn.valid())
                return sol::protected_function_result();

            auto result = fn(std::forward<Args>(args)...);
            if (!result.valid())
            {
                sol::error err = result;
                std::cerr << "[LuaManager] Error calling '" << functionName << "': " << err.what() << std::endl;
            }
            return result;
        }

        /// Check if a global function exists in the loaded script.
        bool hasFunction(const std::string &name) const;

        /// Direct access to the sol::state (for registering bindings).
        sol::state &state() { return _lua; }

    private:
        sol::state _lua;
    };
} // namespace engine::scripting
