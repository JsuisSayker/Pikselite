#include "engine/managers/luaManager.hpp"

namespace engine
{
    LuaManager::LuaManager()
    {
        // Open standard Lua libraries
        _lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table,
                            sol::lib::os);

        // Register a simple log function
        _lua.set_function("log", [](const std::string& msg)
                          { std::cout << "[Lua] " << msg << std::endl; });
    }

    bool LuaManager::loadScript(const std::string& filepath)
    {
        auto result = _lua.safe_script_file(filepath, sol::script_pass_on_error);
        if (!result.valid())
        {
            sol::error err = result;
            std::cerr << "[LuaManager] Failed to load '" << filepath << "': " << err.what()
                      << std::endl;
            return false;
        }
        std::cout << "[LuaManager] Loaded script: " << filepath << std::endl;
        return true;
    }

    bool LuaManager::hasFunction(const std::string& name) const
    {
        sol::object obj = _lua[name];
        return obj.is<sol::protected_function>();
    }
} // namespace engine
