#include <tabSelector/TabSelector.hpp>

TabSelector::TabSelector(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
    _editors.push_back(std::make_unique<ProjectEditor>(graphic, std::make_shared<Core>(graphic)));
    _editors.push_back(std::make_unique<SpriteEditor>(graphic));
}

TabSelector::~TabSelector()
{
}

int TabSelector::run()
{
    while (_graphic->_windowOpen) {
        if (_graphic->tabSelectorData.tabIndex >= _editors.size())
            _graphic->tabSelectorData.tabIndex = 0;
        std::visit([](auto& editor) { editor->run(); }, _editors[_graphic->tabSelectorData.tabIndex]);
    }
    return 0;
}