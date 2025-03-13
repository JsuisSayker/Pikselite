#include <graphic/Graphic.hpp>

namespace graphic
{
    Graphic::Graphic()
    {
        this->_window = SDL_CreateWindow("Pikselite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
        this->_renderer = SDL_CreateRenderer(this->_window, -1, SDL_RENDERER_ACCELERATED);

        if (!this->_window || !this->_renderer)
            throw std::runtime_error("Error: SDL2 failed to initialize.");

        SDL_SetRenderDrawColor(this->_renderer, 255, 255, 255, 255);
        SDL_RenderClear(this->_renderer);
        SDL_RenderPresent(this->_renderer);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO &io = ImGui::GetIO();

        const char *defaultFontPath = "extern/imgui/misc/fonts/Roboto-Medium.ttf";
        ImFont *defaultFont = io.Fonts->AddFontFromFileTTF(defaultFontPath, 16.0f);
        IM_ASSERT(defaultFont != nullptr);
        io.FontDefault = defaultFont;

        ImFontConfig config;
        config.OversampleH = 3;
        const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        const char *fontPath = "extern/icons/fa-solid-900.ttf";
        this->_iconFont = io.Fonts->AddFontFromFileTTF(fontPath, 32.0f, &config, icon_ranges);
        IM_ASSERT(this->_iconFont != nullptr);
        io.Fonts->Build();

        if (_renderer && _window)
        {
            ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
            ImGui_ImplSDLRenderer2_Init(_renderer);
        }
    }

    Graphic::~Graphic()
    {
        SDL_DestroyRenderer(this->_renderer);
        SDL_DestroyWindow(this->_window);
        SDL_Quit();

        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void Graphic::updateWindow()
    {
        SDL_RenderPresent(_renderer);
    }

    void Graphic::clearWindow()
    {
        SDL_SetRenderDrawColor(this->_renderer, 255, 255, 255, 255);
        SDL_RenderClear(this->_renderer);
    }
}
