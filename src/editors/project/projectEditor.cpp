#include <editors/project/projectEditor.hpp>

namespace editors {

    ProjectEditor::ProjectEditor(graphics::Interface* graphicsInterface, graphics::Renderer* renderer, graphics::ImguiInterface* imguiInterface)
        : _graphicsInterface(graphicsInterface), _renderer(renderer), _imguiInterface(imguiInterface) {}

    ProjectEditor::~ProjectEditor() {}

    void ProjectEditor::run(graphics::InputEventType eventType) {
        handleEvents(eventType);

        _renderer->clear();

        // ImGui part
        _imguiInterface->startFrame();
        _imguiInterface->showImGuiDemo();
        _imguiInterface->endFrame(_graphicsInterface->getWindow());

        _renderer->present(_graphicsInterface->getWindow());
    }

    void ProjectEditor::handleEvents(graphics::InputEventType eventType) {
        switch (eventType) {
        case graphics::KEY_W:
            _camera.move(glm::vec2(0.0f, -10.0f));
            break;
        case graphics::KEY_S:
            _camera.move(glm::vec2(0.0f, 10.0f));
            break;
        case graphics::KEY_A:
            _camera.move(glm::vec2(10.0f, 0.0f));
            break;
        case graphics::KEY_D:
            _camera.move(glm::vec2(-10.0f, 0.0f));
            break;
        case graphics::KEY_I:
            _camera.zoomIn(1.1f);
            break;
        case graphics::KEY_O:
            _camera.zoomOut(1.1f);
            break;
        default:
            break;
        }
    }

} // namespace editors