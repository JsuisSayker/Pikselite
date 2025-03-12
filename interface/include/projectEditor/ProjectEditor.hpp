#pragma once

#include <memory>
#include <graphic/Graphic.hpp>

class ProjectEditor
{
public:
    ProjectEditor(std::shared_ptr<graphic::Graphic> graphic);
    ~ProjectEditor();

    int run();
    void moveCamera(graphic::EventType event);
    void zoomCamera(graphic::EventType event);

protected:
private:
    graphic::Camera _camera = graphic::Camera{WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 4};
    std::shared_ptr<graphic::Graphic> _graphic;
};
