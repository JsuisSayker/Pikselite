#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <gtest/gtest.h>

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

    ecs::components::Velocity makeVelocity(float vx, float vy)
    {
        ecs::components::Velocity v{};
        v.enabled = true;
        v.vx = vx;
        v.vy = vy;
        return v;
    }

    ecs::Signature makeMovementSignature(engine::ComponentManager& componentManager)
    {
        ecs::Signature sig{};
        sig.set(componentManager.getComponentType<ecs::components::Transform>());
        sig.set(componentManager.getComponentType<ecs::components::Velocity>());
        return sig;
    }

} // namespace

TEST(ComponentManagerTests, MutationCallbackIsTriggeredOnAddAndRemove)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();

    int callbackCount = 0;
    ecs::EntityID lastEntity = 0;

    componentManager.setEntityMutationCallback(
        [&](ecs::EntityID entityId)
        {
            ++callbackCount;
            lastEntity = entityId;
        });

    constexpr ecs::EntityID entity = 42;
    componentManager.addComponent<ecs::components::Transform>(entity, makeTransform(1.0f, 2.0f));

    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(lastEntity, entity);

    componentManager.removeComponent<ecs::components::Transform>(entity);

    EXPECT_EQ(callbackCount, 2);
    EXPECT_EQ(lastEntity, entity);
}

TEST(EntityManagerTests, CreateDestroyAndReuseEntityId)
{
    engine::EntityManager entityManager;

    const ecs::Entity e1 = entityManager.createEntity();
    const ecs::Entity e2 = entityManager.createEntity();

    EXPECT_TRUE(entityManager.hasEntity(e1.id));
    EXPECT_TRUE(entityManager.hasEntity(e2.id));

    entityManager.destroyEntity(e1);

    EXPECT_FALSE(entityManager.hasEntity(e1.id));
    EXPECT_TRUE(entityManager.hasEntity(e2.id));

    const ecs::Entity e3 = entityManager.createEntity();

    // IDs are recycled from availableIds.
    EXPECT_EQ(e3.id, e1.id);
    EXPECT_TRUE(entityManager.hasEntity(e3.id));
}

TEST(SystemManagerTests, MovementSystemMembershipFollowsSignature)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();

    engine::SystemManager systemManager;
    auto& movement = systemManager.addSystem<ecs::systems::MovementSystem>();
    systemManager.setSignature<ecs::systems::MovementSystem>(
        makeMovementSignature(componentManager));

    constexpr ecs::EntityID entity = 7;

    ecs::Signature onlyTransform{};
    onlyTransform.set(componentManager.getComponentType<ecs::components::Transform>());
    systemManager.entitySignatureChanged(entity, onlyTransform);
    EXPECT_TRUE(movement.entities.empty());

    ecs::Signature transformAndVelocity = makeMovementSignature(componentManager);
    systemManager.entitySignatureChanged(entity, transformAndVelocity);
    EXPECT_EQ(movement.entities.count(entity), 1u);

    systemManager.entitySignatureChanged(entity, onlyTransform);
    EXPECT_TRUE(movement.entities.empty());
}

TEST(SystemManagerTests, SystemWithoutConfiguredSignatureDoesNotMatch)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();

    engine::SystemManager systemManager;
    auto& movement = systemManager.addSystem<ecs::systems::MovementSystem>();

    constexpr ecs::EntityID entity = 9;
    systemManager.entitySignatureChanged(entity, makeMovementSignature(componentManager));

    EXPECT_TRUE(movement.entities.empty());
}

TEST(SystemManagerTests, UpdateAffectsOnlyMatchingEntities)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();

    engine::SystemManager systemManager;
    auto& movement = systemManager.addSystem<ecs::systems::MovementSystem>();
    systemManager.setSignature<ecs::systems::MovementSystem>(
        makeMovementSignature(componentManager));

    constexpr ecs::EntityID moving = 1;
    constexpr ecs::EntityID staticEntity = 2;

    componentManager.addComponent<ecs::components::Transform>(moving, makeTransform(0.0f, 0.0f));
    componentManager.addComponent<ecs::components::Velocity>(moving, makeVelocity(3.0f, -1.0f));

    componentManager.addComponent<ecs::components::Transform>(staticEntity,
                                                              makeTransform(10.0f, 20.0f));

    systemManager.entitySignatureChanged(moving, makeMovementSignature(componentManager));

    ecs::Signature onlyTransform{};
    onlyTransform.set(componentManager.getComponentType<ecs::components::Transform>());
    systemManager.entitySignatureChanged(staticEntity, onlyTransform);

    systemManager.update(2.0, componentManager);

    const auto& movedTransform = componentManager.getComponent<ecs::components::Transform>(moving);
    EXPECT_FLOAT_EQ(movedTransform.x, 6.0f);
    EXPECT_FLOAT_EQ(movedTransform.y, -2.0f);

    const auto& staticTransform =
        componentManager.getComponent<ecs::components::Transform>(staticEntity);
    EXPECT_FLOAT_EQ(staticTransform.x, 10.0f);
    EXPECT_FLOAT_EQ(staticTransform.y, 20.0f);

    EXPECT_EQ(movement.entities.count(moving), 1u);
    EXPECT_EQ(movement.entities.count(staticEntity), 0u);
}
