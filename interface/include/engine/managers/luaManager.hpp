#pragma once

#include <string>
#include <iostream>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace engine
{

    /**
     * @brief The LuaManager class is responsible for managing the Lua scripting environment using the sol2 library.
     * 
     */
    class LuaManager
    {
    public:

        /**
         * @brief Construct a new Lua Manager object
         * 
         */
        LuaManager();
        ~LuaManager() = default;

        /**
         * @brief Load and execute a .lua file. Functions defined in it become callable.
         * 
         */
        bool loadScript(const std::string &filepath);

        /**
         * @brief Call a Lua function by name with arbitrary args (forwarded via sol2).
         * 
         * @tparam Args 
         * @param functionName 
         * @param args 
         * @return sol::protected_function_result 
         */
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

        /**
         * @brief Check if a global function exists in the loaded script.
         * 
         * @param name 
         * @return true 
         * @return false 
         */
        bool hasFunction(const std::string &name) const;

        /**
         * @brief Direct access to the sol::state (for registering bindings).
         * 
         * @return sol::state& 
         */
        sol::state &state() { return _lua; }

    private:
        // The sol::state object manages the Lua environment and is used to execute scripts and call functions.
        sol::state _lua;
    };
} // namespace engine::scripting
