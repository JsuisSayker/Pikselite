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
        this->_iconFont = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &config, icon_ranges);
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

    void Graphic::saveSprite(std::vector<Pixel> pixels, std::string filename)
    {
        SDL_Surface *surface = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0, 0, 0, 0);

        SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 255, 255, 255, 255));

        for (const auto &pixel : pixels)
        {
            SDL_Rect rect = {static_cast<int>(pixel.position.x), static_cast<int>(pixel.position.y), 1, 1};
            SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, pixel.color.r, pixel.color.g, pixel.color.b, pixel.color.a));
        }

        if (IMG_SavePNG(surface, filename.c_str()) != 0)
        {
            std::cerr << "Erreur lors de l'enregistrement de l'image PNG : " << IMG_GetError() << std::endl;
        }

        SDL_FreeSurface(surface);
    }

    void Graphic::createExternalAttributeFile(const std::string &filename, const std::vector<Pixel> &pixels)
    {

        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

        rapidjson::Value pixelArray(rapidjson::kArrayType);

        for (const auto &pixel : pixels)
        {
            rapidjson::Value pixelObj(rapidjson::kObjectType);

            rapidjson::Value posObj(rapidjson::kObjectType);
            posObj.AddMember("x", pixel.position.x, allocator);
            posObj.AddMember("y", pixel.position.y, allocator);
            pixelObj.AddMember("position", posObj, allocator);

            rapidjson::Value colorObj(rapidjson::kObjectType);
            colorObj.AddMember("r", static_cast<int>(pixel.color.r), allocator);
            colorObj.AddMember("g", static_cast<int>(pixel.color.g), allocator);
            colorObj.AddMember("b", static_cast<int>(pixel.color.b), allocator);
            colorObj.AddMember("a", static_cast<int>(pixel.color.a), allocator);
            pixelObj.AddMember("color", colorObj, allocator);

            pixelArray.PushBack(pixelObj, allocator);
        }

        doc.AddMember("pixels", pixelArray, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::ofstream ofs(filename);
        if (!ofs)
        {
            throw std::runtime_error("Could not open file: " + filename);
        }
        ofs << buffer.GetString();
        ofs.close();
    }

    std::pair<std::string, std::vector<Pixel>> Graphic::loadSpriteFromJSON(const std::string &filename)
    {
        std::vector<Pixel> pixels;
        FILE *fp = std::fopen(filename.c_str(), "rb");
        if (!fp)
        {
            std::perror("Erreur lors de l'ouverture du fichier");
            throw std::runtime_error("Could not open file: " + filename);
        }
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        rapidjson::Document doc;
        doc.ParseStream(is);
        std::fclose(fp);

        if (!doc.IsObject())
        {
            std::cerr << "Le document JSON n'est pas un objet." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        if (!doc.HasMember("pixels"))
        {
            std::cerr << "Le document ne contient pas le membre 'pixels'." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        const rapidjson::Value &pixelsArray = doc["pixels"];
        if (!pixelsArray.IsArray())
        {
            std::cerr << "Le membre 'pixels' n'est pas un tableau." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        for (rapidjson::SizeType i = 0; i < pixelsArray.Size(); ++i)
        {
            const rapidjson::Value &pixelObj = pixelsArray[i];
            if (!pixelObj.HasMember("position") || !pixelObj.HasMember("color"))
                continue;

            const rapidjson::Value &posObj = pixelObj["position"];
            const rapidjson::Value &colorObj = pixelObj["color"];

            if (!posObj.IsObject() || !colorObj.IsObject())
                continue;

            Pixel p;
            p.position.x = posObj["x"].GetDouble();
            p.position.y = posObj["y"].GetDouble();
            p.color.r = colorObj["r"].GetInt();
            p.color.g = colorObj["g"].GetInt();
            p.color.b = colorObj["b"].GetInt();
            p.color.a = colorObj["a"].GetInt();

            pixels.push_back(p);
        }
        std::pair<std::string, std::vector<Pixel>> sprite = std::make_pair(filename, pixels);
        return sprite;
    }
}
