#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

namespace
{
    ecs::components::Transform makeTransform(float x, float y)
    {
        ecs::components::Transform t{};
        t.enabled = true;
        t.x = x;
        t.y = y;
        t.rotation = 0.0f;
        t.scaleX = 1.0f;
        t.scaleY = 1.0f;
        t.prevX = 0.0f;
        t.prevY = 0.0f;
        return t;
    }
} // namespace

TEST(EcsResilienceTests, RemoveNonExistentComponent)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    constexpr ecs::EntityID entity = 1;
    // Should not throw for an entity that never had the component
    EXPECT_NO_THROW(componentManager.removeComponent<ecs::components::Transform>(entity));
}

TEST(EcsResilienceTests, GetNonExistentComponentThrows)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    constexpr ecs::EntityID entity = 999;
    // getComponent on entity with no such component should throw
    EXPECT_THROW(componentManager.getComponent<ecs::components::Transform>(entity),
                 std::out_of_range);
}

TEST(EcsResilienceTests, HasComponentForNonExistentEntity)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    constexpr ecs::EntityID entity = 12345;
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(entity));
}

TEST(EcsResilienceTests, DestroyNonExistentEntity)
{
    engine::EntityManager entityManager;
    // destroyEntity takes an ecs::Entity struct
    const ecs::Entity nonExistent(9999);
    EXPECT_NO_THROW(entityManager.destroyEntity(nonExistent));
}

TEST(EcsResilienceTests, HasEntityWithNonExistentId)
{
    engine::EntityManager entityManager;
    EXPECT_FALSE(entityManager.hasEntity(9999));
}

TEST(EcsResilienceTests, DestroyEntityRemovesSignature)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();

    ecs::Entity entity = entityManager.createEntity();
    const ecs::EntityID id = entity.id;

    componentManager.addComponent<ecs::components::Transform>(id, makeTransform(1.0f, 2.0f));
    EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(id));

    entityManager.destroyEntity(entity);
    componentManager.entityDestroyed(id);

    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(id));
}

TEST(EcsResilienceTests, EntityManagerDestroyAlreadyDestroyed)
{
    engine::EntityManager entityManager;

    ecs::Entity entity = entityManager.createEntity();
    entityManager.destroyEntity(entity); // first destroy
    EXPECT_NO_THROW(entityManager.destroyEntity(entity)); // second destroy should be safe
}

TEST(EcsResilienceTests, SetSignatureForUnregisteredComponentDoesNotThrow)
{
    engine::SystemManager systemManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    // A signature with a bit for an unregistered component type
    // (32 is beyond MAX_COMPONENT_TYPES bitset size — test the last valid bit instead)
    ecs::Signature sig;
    sig.set(31); // last valid bit in std::bitset<32>, not used by any registered component

    // This should not crash; systems simply won't match such entities
    constexpr ecs::EntityID entity = 42;
    EXPECT_NO_THROW(systemManager.entitySignatureChanged(entity, sig));
}

TEST(EcsResilienceTests, EntityDestroyedSignatureCleanup)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;

    componentManager.registerComponent<ecs::components::Transform>();

    ecs::Entity entity = entityManager.createEntity();
    const ecs::EntityID id = entity.id;

    componentManager.addComponent<ecs::components::Transform>(id, makeTransform(5.0f, 10.0f));
    EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(id));

    // Destroy entity through all managers
    entityManager.destroyEntity(entity);
    componentManager.entityDestroyed(id);
    systemManager.entityDestroyed(id);

    EXPECT_FALSE(entityManager.hasEntity(id));
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(id));
}

TEST(EcsResilienceTests, SystemManagerUpdateWithNoSystems)
{
    engine::SystemManager systemManager;
    engine::ComponentManager componentManager;

    // update with no systems registered should be a no-op
    EXPECT_NO_THROW(systemManager.update(1.0, componentManager));
}
