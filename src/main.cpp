#include <tabSelector/TabSelector.hpp>
#include <home/Home.hpp>
#include <memory>
#include <stdio.h>

int main()
{
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>();
    std::unique_ptr<TabSelector> editor = std::make_unique<TabSelector>(graphic);
    std::unique_ptr<Home> home = std::make_unique<Home>(graphic);

    home->run();
    return editor->run();
}
