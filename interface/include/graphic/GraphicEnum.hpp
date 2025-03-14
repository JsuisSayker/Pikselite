#pragma once
#include <cstdint>
#include <iostream>
#include <vector>
#include <variant>

namespace graphic
{
    enum class EventType : uint8_t
    {
        MOUSE_WHEEL_UP,
        MOUSE_WHEEL_DOWN,
        WINDOW_CLOSE,
        MOUSE_CLICK_LEFT,
        MOUSE_CLICK_RIGHT,
        MOUSE_DRAG_LEFT,
        MOUSE_DRAG_RIGHT,
        MOUSE_MOVE,
        KEYBOARD_PRESS,
        KEY_ARROW_UP,
        KEY_ARROW_DOWN,
        KEY_ARROW_LEFT,
        KEY_ARROW_RIGHT,
        KEY_TAB,
        KEY_A,
        KEY_B,
        KEY_C,
        KEY_D,
        KEY_E,
        KEY_F,
        KEY_G,
        KEY_H,
        KEY_I,
        KEY_J,
        KEY_K,
        KEY_L,
        KEY_M,
        KEY_N,
        KEY_O,
        KEY_P,
        KEY_Q,
        KEY_R,
        KEY_S,
        KEY_T,
        KEY_U,
        KEY_V,
        KEY_W,
        KEY_X,
        KEY_Y,
        KEY_Z,
        NONE,
    };

    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    struct Position
    {
        float x;
        float y;
    };

    struct Camera
    {
        Position position;
        int zoom;
    };

    struct Border
    {
        int radius;
        int thickness;
        Color color;
    };

    struct Rectangle
    {
        int width;
        int height;
        Color color;
        Border border;
        Position position;
    };

    struct light
    {
        int radius;
        int intensity;
    };

    struct solid
    {
    };

    struct liquid
    {
        int viscosity;
    };

    struct Pixel
    {
        bool isSelected;

        Color color;
        Position position;
        std::vector<std::variant<liquid, solid, light>> attributes;
    };

    struct SpriteEditorData
    {
        Color selectedColor;
        Color defaultColor;

        std::variant<light, solid, liquid> selectedAttribute;
        std::variant<light, solid, liquid> defaultAttribute;

        bool showColorSelector;
        bool showPixelEditorSidebar;
        bool showGrid;
        bool autoLink;
        bool showLightOptions = false;
        bool showSolidOptions = false;
        bool showLiquidOptions = false;
        bool resetCamera = false;
    };

    struct ProjectEditorData
    {
        bool showFileExplorer;
        bool showDirectoryChooser;
        bool showLayer;
        bool showGameAssets;
        bool showPixelLinker;
        bool showScenes;
        std::string spritePath;
        std::string folderPath;
    };

    struct TabSelectorData
    {
        int tabIndex = 0;
    };

    struct Sprite
    {
        bool isSelected;

        Position position;
        std::string name;
        std::vector<Pixel> pixels;
    };
}