#pragma once

#include <memory>
#include <graphic/Graphic.hpp>

class Editor {
    public:
        Editor(std::shared_ptr<graphic::Graphic> graphic);
        ~Editor();

        void run();

    protected:
    private:
        std::shared_ptr<graphic::Graphic> _graphic;
};
