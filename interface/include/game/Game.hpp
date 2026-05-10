#pragma once

#include <build/BuildSettings.hpp>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/systems/scriptSystem.hpp>
#include <engine/events/eventBus.hpp>
#include <engine/events/events.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <engine/physics/boxWorld.hpp>
#include <engine/pixels/simulation/element.hpp>
#include <engine/pixels/simulation/simulation.hpp>
#include <engine/pixels/pixelEnum.hpp>
#include <engine/time.hpp>
#include <graphics/renderer/camera.hpp>
#include <graphics/renderer/renderer.hpp>

#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine
{
    class Game
    {
      public:
        Game(int width, int height, const std::string& title = "Pikselite Game");
        ~Game();

        bool loadScene(const std::string& filename);
        void run();

        bool isRunning() const { return _running; }
        void quit() { _running = false; }

      private:
        bool _running = true;

        graphics::Camera2D _camera;
        engine::physics::BoxWorld _boxWorld;
        ChunkGrid _chunkGrid;
        Simulation _pixelSimulation;

        engine::EntityManager _entityManager;
        engine::ComponentManager _componentManager;
        engine::SystemManager _systemManager;
        engine::events::EventBus _eventBus;

        std::vector<Pixel::GameObject> _gameObjects;
        uint32_t _gameObjectCounter = 1;
        std::unordered_map<Pixel::GameObjectID, ecs::EntityID> _gameObjectToEntity;
        std::unordered_map<Pixel::GameObjectID, std::vector<Element::Vec2i>> _gameObjectOccupiedCells;

        SDL_Window* _window = nullptr;
        SDL_GLContext _glContext = nullptr;
        graphics::Renderer* _renderer = nullptr;

        Timer _timer;
        float _accumulator = 0.0f;
        const float _fixedDt = 1.0f / 60.0f;

        void initSDL(int width, int height, const std::string& title);
        void shutdownSDL();

        void initECS();
        void loadGameObjectsIntoECS();
        void syncGameObjectPixelsFromPhysics();
        void refreshEntitySignature(ecs::EntityID entityId);

        std::vector<graphics::Pixel> buildRenderPixels(const ChunkGrid& grid) const;
        void handleEvents();
        void update(float deltaTime);
        void render();
    };
}
