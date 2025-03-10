#include <editor/Editor.hpp>
#include <memory>
#include <stdio.h>

int main()
{
    graphic::Position position;
    graphic::EventType event;
    std::shared_ptr<graphic::Graphic> graphic = std::make_shared<graphic::Graphic>();
    std::unique_ptr<Editor> editor = std::make_unique<Editor>(graphic);

    return editor->run();
}
