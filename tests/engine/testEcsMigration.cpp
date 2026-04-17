#include <gtest/gtest.h>

#include <unordered_map>
#include <vector>

#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>

#include <engine/pixels/pixelEnum.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>

namespace {

void migrateGameObjectsIntoECS(
    const std::vector<Pixel::GameObject>& gameObjects,
    std::unordered_map<Pixel::GameObjectID, ecs::EntityID>& gameObjectToEntity,
    engine::EntityManager& entityManager,
    engine::ComponentManager& componentManager,
    engine::SystemManager& systemManager)
{
    const auto previousMapping = gameObjectToEntity;
    gameObjectToEntity.clear();

    for (const auto& go : gameObjects)
    {
        ecs::Entity entity = entityManager.createEntity();
        const ecs::EntityID newEntityId = entity.id;

        const auto previousIt = previousMapping.find(go.id);
        const bool existedBefore = previousIt != previousMapping.end();

        if (existedBefore)
        {
            componentManager.copyEntityComponents(previousIt->second, newEntityId);
        }

        if (componentManager.hasComponent<ecs::components::GameObjectLink>(newEntityId))
        {
            auto& link = componentManager.getComponent<ecs::components::GameObjectLink>(newEntityId);
            link.gameObjectId = go.id;
            link.pixelEntities = go.pixels;
        }

        gameObjectToEntity[go.id] = newEntityId;
    }

    for (const auto& [goId, oldEntityId] : previousMapping)
    {
        componentManager.entityDestroyed(oldEntityId);
        systemManager.entityDestroyed(oldEntityId);
        entityManager.destroyEntity(ecs::Entity(oldEntityId));
    }
}

ecs::components::Transform makeTransform(float x, float y)
{
    ecs::components::Transform t{};
    t.enabled = true;
    t.x = x;
    t.y = y;
    t.rotation = 15.0f;
    t.scaleX = 2.0f;
    t.scaleY = 3.0f;
    t.prevX = -1.0f;
    t.prevY = -2.0f;
    return t;
}

ecs::components::Sprite makeSprite(const std::string& path)
{
    ecs::components::Sprite s{};
    s.enabled = true;
    s.texturePath = path;
    s.width = 42.0f;
    s.height = 24.0f;
    s.loaded = true;
    return s;
}

Pixel::GameObject makeGameObject(Pixel::GameObjectID id)
{
    Pixel::GameObject go{};
    go.id = id;
    go.name = "GO_" + std::to_string(id);
    go.isActive = true;
    go.pixels.push_back(Element::Pixel{Element::SAND});
    return go;
}

} // namespace

TEST(EcsMigrationTests, ExistingEntityComponentsAreCopiedToMigratedEntity)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;

    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::Sprite>();
    componentManager.registerComponent<ecs::components::GameObjectLink>();

    std::unordered_map<Pixel::GameObjectID, ecs::EntityID> mapping;

    const Pixel::GameObject go = makeGameObject(1);

    const ecs::Entity oldEntity = entityManager.createEntity();
    mapping[go.id] = oldEntity.id;

    componentManager.addComponent<ecs::components::Transform>(oldEntity.id, makeTransform(10.0f, 20.0f));
    componentManager.addComponent<ecs::components::Sprite>(oldEntity.id, makeSprite("assets/custom.png"));

    ecs::components::GameObjectLink link{};
    link.gameObjectId = go.id;
    link.pixelEntities = {Element::Pixel{Element::WATER}};
    componentManager.addComponent<ecs::components::GameObjectLink>(oldEntity.id, link);

    migrateGameObjectsIntoECS({go}, mapping, entityManager, componentManager, systemManager);

    ASSERT_EQ(mapping.size(), 1u);
    const ecs::EntityID newEntityId = mapping.at(go.id);

    EXPECT_TRUE(entityManager.hasEntity(newEntityId));
    EXPECT_FALSE(entityManager.hasEntity(oldEntity.id));

    ASSERT_TRUE(componentManager.hasComponent<ecs::components::Transform>(newEntityId));
    const auto& t = componentManager.getComponent<ecs::components::Transform>(newEntityId);
    EXPECT_FLOAT_EQ(t.x, 10.0f);
    EXPECT_FLOAT_EQ(t.y, 20.0f);
    EXPECT_FLOAT_EQ(t.rotation, 15.0f);

    ASSERT_TRUE(componentManager.hasComponent<ecs::components::Sprite>(newEntityId));
    const auto& s = componentManager.getComponent<ecs::components::Sprite>(newEntityId);
    EXPECT_EQ(s.texturePath, "assets/custom.png");
    EXPECT_FLOAT_EQ(s.width, 42.0f);
    EXPECT_FLOAT_EQ(s.height, 24.0f);

    ASSERT_TRUE(componentManager.hasComponent<ecs::components::GameObjectLink>(newEntityId));
    const auto& migratedLink = componentManager.getComponent<ecs::components::GameObjectLink>(newEntityId);
    EXPECT_EQ(migratedLink.gameObjectId, go.id);
    EXPECT_EQ(migratedLink.pixelEntities.size(), go.pixels.size());
}

TEST(EcsMigrationTests, NewGameObjectGetsNoImplicitDefaultComponents)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;

    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::Sprite>();
    componentManager.registerComponent<ecs::components::GameObjectLink>();

    std::unordered_map<Pixel::GameObjectID, ecs::EntityID> mapping;

    const Pixel::GameObject go = makeGameObject(2);

    migrateGameObjectsIntoECS({go}, mapping, entityManager, componentManager, systemManager);

    ASSERT_EQ(mapping.size(), 1u);
    const ecs::EntityID entityId = mapping.at(go.id);

    EXPECT_TRUE(entityManager.hasEntity(entityId));
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(entityId));
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Velocity>(entityId));
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Sprite>(entityId));
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::GameObjectLink>(entityId));
}

TEST(EcsMigrationTests, RemovedGameObjectsAreDestroyedFromEcs)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;

    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::Sprite>();
    componentManager.registerComponent<ecs::components::GameObjectLink>();

    std::unordered_map<Pixel::GameObjectID, ecs::EntityID> mapping;

    const Pixel::GameObject go1 = makeGameObject(10);
    const Pixel::GameObject go2 = makeGameObject(11);

    const ecs::Entity e1 = entityManager.createEntity();
    const ecs::Entity e2 = entityManager.createEntity();

    mapping[go1.id] = e1.id;
    mapping[go2.id] = e2.id;

    componentManager.addComponent<ecs::components::Transform>(e1.id, makeTransform(1.0f, 1.0f));
    componentManager.addComponent<ecs::components::Transform>(e2.id, makeTransform(2.0f, 2.0f));

    // Only keep go1 in the next editor snapshot.
    migrateGameObjectsIntoECS({go1}, mapping, entityManager, componentManager, systemManager);

    ASSERT_EQ(mapping.size(), 1u);
    EXPECT_EQ(mapping.count(go1.id), 1u);
    EXPECT_EQ(mapping.count(go2.id), 0u);

    // e2 should have been removed from ECS as its GameObject disappeared.
    EXPECT_FALSE(entityManager.hasEntity(e2.id));
}
