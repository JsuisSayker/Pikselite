#include <spriteEditor/SpriteEditor.hpp>

SpriteEditor::SpriteEditor(std::shared_ptr<graphic::Graphic> graphic) : _graphic(graphic)
{
}

SpriteEditor::~SpriteEditor()
{
}

void SpriteEditor::checkAttribute()
{
    bool noLight = true;
    bool noSolid = true;
    bool noLiquid = true;

    if (_graphic->editorData.lightEnabled)
    {
        for (std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute : _graphic->editorData.defaultAttributes)
        {
            if (std::holds_alternative<graphic::light>(attribute))
            {
                attribute = graphic::light{_graphic->lightData.radius, _graphic->lightData.intensity};
                noLight = false;
            }
        }
        if (noLight)
        {
            _graphic->editorData.defaultAttributes.push_back(graphic::light{_graphic->lightData.radius, _graphic->lightData.intensity});
        }
    }
    else
    {
        _graphic->editorData.defaultAttributes.erase(
            std::remove_if(
                _graphic->editorData.defaultAttributes.begin(),
                _graphic->editorData.defaultAttributes.end(),
                [](const std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute)
                {
                    return std::holds_alternative<graphic::light>(attribute);
                }),
            _graphic->editorData.defaultAttributes.end());
    }
    if (_graphic->editorData.solidEnabled)
    {
        for (std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute : _graphic->editorData.defaultAttributes)
        {
            if (std::holds_alternative<graphic::solid>(attribute))
            {
                attribute = graphic::solid{};
                noSolid = false;
            }
        }
        if (noSolid)
        {
            _graphic->editorData.defaultAttributes.push_back(graphic::solid{});
        }
    }
    else
    {
        _graphic->editorData.defaultAttributes.erase(
            std::remove_if(
                _graphic->editorData.defaultAttributes.begin(),
                _graphic->editorData.defaultAttributes.end(),
                [](const std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute)
                {
                    return std::holds_alternative<graphic::solid>(attribute);
                }),
            _graphic->editorData.defaultAttributes.end());
    }
    if (_graphic->editorData.liquidEnabled)
    {
        for (std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute : _graphic->editorData.defaultAttributes)
        {
            if (std::holds_alternative<graphic::liquid>(attribute))
            {
                attribute = graphic::liquid{_graphic->liquidData.viscosity};
                noLiquid = false;
            }
        }
        if (noLiquid)
        {
            _graphic->editorData.defaultAttributes.push_back(graphic::liquid{_graphic->liquidData.viscosity});
        }
    }
    else
    {
        _graphic->editorData.defaultAttributes.erase(
            std::remove_if(
                _graphic->editorData.defaultAttributes.begin(),
                _graphic->editorData.defaultAttributes.end(),
                [](const std::variant<graphic::light, graphic::solid, graphic::liquid> &attribute)
                {
                    return std::holds_alternative<graphic::liquid>(attribute);
                }),
            _graphic->editorData.defaultAttributes.end());
    }
}

int SpriteEditor::run()
{
    graphic::EventType event;

    _graphic->isSpriteEditor = true;
    _graphic->isProjectEditor = false;

    while (_graphic->_windowOpen)
    {
        event = _graphic->checkEvent();
        checkAttribute();

        if (event == graphic::EventType::WINDOW_CLOSE)
            return 0;

        if (event == graphic::EventType::KEY_TAB)
        {
            _graphic->tabSelectorData.tabIndex += 1;
            return 0;
        }

        if (!_graphic->projectData.folderPath.empty())
        {
            try
            {
                _graphic->saveSprite(_graphic->_pixels, _graphic->projectData.folderPath);
                _graphic->createExternalAttributeFile(_graphic->projectData.folderPath, _graphic->_pixels);
                _graphic->projectData.folderPath.clear();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
        if (_graphic->editorData.resetCamera)
        {
            _camera = graphic::Camera{0, 0, 4};
            _graphic->editorData.resetCamera = false;
        }

        checkMouseEvents(event);
        moveCamera(event);
        zoomCamera(event);

        _graphic->clearWindow();

        _graphic->drawPixels(_camera);
        if (_graphic->editorData.showGrid)
            _graphic->drawGrid(_camera);
        _graphic->drawInterface(_camera);

        _graphic->updateWindow();
    }
    return 0;
}