#pragma once

#include <memory>
#include <graphic/Graphic.hpp>

class Home {
    public:
        Home(std::shared_ptr<graphic::Graphic> graphic);
        ~Home();

        int run();

    protected:
    private:
        std::shared_ptr<graphic::Graphic> _graphic;
};
