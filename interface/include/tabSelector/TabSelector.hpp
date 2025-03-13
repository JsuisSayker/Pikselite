#pragma once

#include <projectEditor/ProjectEditor.hpp>
#include <spriteEditor/SpriteEditor.hpp>
#include <core/Core.hpp>

#include <memory>
#include <variant>

class TabSelector {
    public:
        TabSelector(std::shared_ptr<graphic::Graphic> graphic);
        ~TabSelector();

        int run();

    protected:
    private:
        std::shared_ptr<graphic::Graphic> _graphic;
        std::vector<std::variant<std::unique_ptr<ProjectEditor>, std::unique_ptr<SpriteEditor>>> _editors;
};
