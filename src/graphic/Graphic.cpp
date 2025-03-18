#include <graphic/Graphic.hpp>

namespace graphic
{
    Graphic::Graphic(bool interface)
    {
        showInterface = interface;
        if (interface)
            this->_window = SDL_CreateWindow("Pikselite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
        else
            this->_window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        this->_renderer = SDL_CreateRenderer(this->_window, -1, SDL_RENDERER_ACCELERATED);

        if (!this->_window || !this->_renderer)
            throw std::runtime_error("Error: SDL2 failed to initialize.");

        SDL_SetRenderDrawColor(this->_renderer, 255, 255, 255, 255);
        SDL_RenderClear(this->_renderer);
        SDL_RenderPresent(this->_renderer);

        if (interface)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGuiIO &io = ImGui::GetIO();

            // const char *defaultFontPath = "extern/imgui/misc/fonts/Roboto-Medium.ttf";
            // ImFont *defaultFont = io.Fonts->AddFontFromFileTTF(defaultFontPath, 16.0f);
            // IM_ASSERT(defaultFont != nullptr);
            // io.FontDefault = defaultFont;

            ImFontConfig config;
            config.MergeMode = true;
            config.PixelSnapH = true;
            config.OversampleH = 3;
            // const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
            // const char *fontPath = "extern/icons/fa-solid-900.ttf";
            // this->_iconFont = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &config, icon_ranges);
            // IM_ASSERT(this->_iconFont != nullptr);
            // io.Fonts->Build();

            if (_renderer && _window)
            {
                ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
                ImGui_ImplSDLRenderer2_Init(_renderer);
            }
        }
        if (TTF_Init() == -1) {
            std::cerr << "Erreur TTF_Init : " << TTF_GetError() << std::endl;
        }

        m_font = TTF_OpenFont("extern/fonts/pixely.ttf", 24);
        if (!m_font) {
            std::cerr << "Erreur lors du chargement de la police : " << TTF_GetError() << std::endl;
        }
    }

    Graphic::~Graphic()
    {
        SDL_DestroyRenderer(this->_renderer);
        SDL_DestroyWindow(this->_window);

        if (showInterface)
        {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        }
        if (m_font)
        {
            TTF_CloseFont(m_font);
        }
        TTF_Quit();
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
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
        if (!surface)
        {
            std::cerr << "Error while creating surface: " << SDL_GetError() << std::endl;
            return;
        }

        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

        SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

        int offsetX = WINDOW_WIDTH / 2;
        int offsetY = WINDOW_HEIGHT / 2;

        for (const auto &pixel : pixels)
        {
            int sdlX = static_cast<int>(pixel.position.x) + offsetX;
            int sdlY = offsetY + static_cast<int>(pixel.position.y);

            if (sdlX >= 0 && sdlX < WINDOW_WIDTH && sdlY >= 0 && sdlY < WINDOW_HEIGHT)
            {
                SDL_Rect rect = {sdlX, sdlY, 1, 1};
                Uint32 color = SDL_MapRGBA(surface->format, pixel.color.r, pixel.color.g, pixel.color.b, pixel.color.a);
                SDL_FillRect(surface, &rect, color);
            }
        }

        if (IMG_SavePNG(surface, filename.c_str()) != 0)
        {
            std::cerr << "Error while saving image: " << SDL_GetError() << std::endl;
        }

        SDL_FreeSurface(surface);
    }

    void Graphic::createExternalAttributeFile(const std::string &filename, const std::vector<Pixel> &pixels)
    {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

        rapidjson::Value pixelArray(rapidjson::kArrayType);

        // remove the file extension
        std::string attributeFilename = filename.substr(0, filename.find_last_of('.')) + ".json";

        for (const auto &pixel : pixels)
        {
            rapidjson::Value pixelObj(rapidjson::kObjectType);

            rapidjson::Value posObj(rapidjson::kObjectType);
            posObj.AddMember("x", pixel.position.x, allocator);
            posObj.AddMember("y", pixel.position.y, allocator);
            pixelObj.AddMember("position", posObj, allocator);

            rapidjson::Value attributesArray(rapidjson::kArrayType);
            for (const auto &attribute : pixel.attributes)
            {
                rapidjson::Value attributeObj(rapidjson::kObjectType);
                if (std::holds_alternative<light>(attribute))
                {
                    const light &l = std::get<light>(attribute);
                    attributeObj.AddMember("radius", l.radius, allocator);
                    attributeObj.AddMember("intensity", l.intensity, allocator);
                }
                else if (std::holds_alternative<solid>(attribute))
                {
                    // do nothing
                }
                else if (std::holds_alternative<liquid>(attribute))
                {
                    const liquid &l = std::get<liquid>(attribute);
                    attributeObj.AddMember("viscosity", l.viscosity, allocator);
                }
                attributesArray.PushBack(attributeObj, allocator);
            }

            pixelObj.AddMember("attributes", attributesArray, allocator);

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

        std::ofstream ofs(attributeFilename);
        if (!ofs)
        {
            throw std::runtime_error("Could not open file: " + attributeFilename);
        }
        ofs << buffer.GetString();
        ofs.close();
    }

    Sprite Graphic::loadSpriteFromJSON(const std::string &filename, bool defaultUsage, Position actualPosition)
    {
        std::vector<Pixel> pixels;
        FILE *fp = std::fopen(filename.c_str(), "rb");
        if (!fp)
        {
            std::perror("Error while opening file");
            throw std::runtime_error("Could not open file: " + filename);
        }
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        rapidjson::Document doc;
        doc.ParseStream(is);
        std::fclose(fp);

        if (!doc.IsObject())
        {
            std::cerr << "The JSON document is not an object." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        if (!doc.HasMember("pixels"))
        {
            std::cerr << "The JSON document does not have a 'pixels' member." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        const rapidjson::Value &pixelsArray = doc["pixels"];
        if (!pixelsArray.IsArray())
        {
            std::cerr << "The 'pixels' member is not an array." << std::endl;
            throw std::runtime_error("Invalid JSON file: " + filename);
        }

        for (rapidjson::SizeType i = 0; i < pixelsArray.Size(); ++i)
        {
            const rapidjson::Value &pixelObj = pixelsArray[i];
            if (!pixelObj.HasMember("position") || !pixelObj.HasMember("color"))
                continue;

            const rapidjson::Value &posObj = pixelObj["position"];
            const rapidjson::Value &colorObj = pixelObj["color"];
            const rapidjson::Value &attributesArray = pixelObj["attributes"];

            if (!posObj.IsObject() || !colorObj.IsObject() || !attributesArray.IsArray())
                continue;

            Pixel p;
            if (defaultUsage)
            {
                p.position.x = posObj["x"].GetDouble();
                p.position.y = posObj["y"].GetDouble();
            }
            else
            {
                p.position.x = actualPosition.x + posObj["x"].GetDouble();
                p.position.y = actualPosition.y + posObj["y"].GetDouble();
            }

            p.color.r = colorObj["r"].GetInt();
            p.color.g = colorObj["g"].GetInt();
            p.color.b = colorObj["b"].GetInt();
            p.color.a = colorObj["a"].GetInt();

            for (rapidjson::SizeType j = 0; j < attributesArray.Size(); ++j)
            {
                const rapidjson::Value &attributeObj = attributesArray[j];
                if (!attributeObj.IsObject())
                    continue;

                if (attributeObj.HasMember("radius") && attributeObj.HasMember("intensity"))
                {
                    light l;
                    l.radius = attributeObj["radius"].GetDouble();
                    l.intensity = attributeObj["intensity"].GetDouble();
                    p.attributes.push_back(l);
                }
                else if (attributeObj.HasMember("viscosity"))
                {
                    liquid l;
                    l.viscosity = attributeObj["viscosity"].GetDouble();
                    p.attributes.push_back(l);
                }
                else
                {
                    solid s;
                    p.attributes.push_back(s);
                }
            }
            pixels.push_back(p);
        }
        Sprite sprite = Sprite{false, Position{0, 0}, filename, pixels};
        return sprite;
    }
}
