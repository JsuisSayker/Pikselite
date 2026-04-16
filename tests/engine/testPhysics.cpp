#include <gtest/gtest.h>

#include <box2d/box2d.h>

#include <engine/physics/boxWorld.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/ecs/systems/physicsSystem.hpp>
#include <engine/ecs/systems/movementSystem.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>

namespace {

ecs::components::Transform makeTransform(float x, float y)
{
    ecs::components::Transform t{};
    t.enabled = true;
    t.x = x;
    t.y = y;
    t.rotation = 0.0f;
    t.scaleX = 1.0f;
    t.scaleY = 1.0f;
    t.prevX = x;
    t.prevY = y;
    return t;
}

ecs::components::PhysicsBody makePhysicsBody(bool enabled = true)
{
    ecs::components::PhysicsBody body{};
    body.enabled = enabled;
    body.bodyId = b2_nullBodyId;
    body.bodyType = b2_dynamicBody;
    body.fixedRotation = false;
    body.density = 1.0f;
    body.friction = 0.4f;
    body.restitution = 0.1f;
    return body;
}

} // namespace

TEST(BoxWorldTests, InitStepAndShutdownLifecycle)
{
    engine::physics::BoxWorld world;
    EXPECT_FALSE(world.isValid());

    world.init({0.0f, 0.0f});
    EXPECT_TRUE(world.isValid());
    EXPECT_TRUE(B2_IS_NON_NULL(world.getWorldId()));

    world.step(1.0f / 60.0f, 4);

    world.shutdown();
    EXPECT_FALSE(world.isValid());
    EXPECT_TRUE(B2_IS_NULL(world.getWorldId()));
}

TEST(PhysicsSystemTests, DisabledPhysicsBodyDoesNotCreateBox2DBody)
{
    engine::physics::BoxWorld world({0.0f, 0.0f});

    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();

    constexpr ecs::EntityID entity = 100;
    componentManager.addComponent<ecs::components::Transform>(entity, makeTransform(2.0f, 3.0f));
    componentManager.addComponent<ecs::components::PhysicsBody>(entity, makePhysicsBody(false));

    ecs::systems::PhysicsSystem system(&world);
    system.entities.insert(entity);

    system.update(1.0 / 60.0, componentManager);

    const auto &physics = componentManager.getComponent<ecs::components::PhysicsBody>(entity);
    EXPECT_TRUE(B2_IS_NULL(physics.bodyId));
}

TEST(PhysicsSystemTests, CreatesBodyAndSynchronizesTransformFromSimulation)
{
    engine::physics::BoxWorld world({0.0f, 0.0f});

    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();
    componentManager.registerComponent<ecs::components::Sprite>();

    constexpr ecs::EntityID entity = 101;
    componentManager.addComponent<ecs::components::Transform>(entity, makeTransform(5.0f, 8.0f));
    componentManager.addComponent<ecs::components::PhysicsBody>(entity, makePhysicsBody(true));

    ecs::components::Sprite sprite{};
    sprite.width = 16.0f;
    sprite.height = 24.0f;
    componentManager.addComponent<ecs::components::Sprite>(entity, sprite);

    ecs::systems::PhysicsSystem system(&world);
    system.entities.insert(entity);

    // First update creates the body from current ECS component data.
    system.update(1.0 / 60.0, componentManager);

    auto &physics = componentManager.getComponent<ecs::components::PhysicsBody>(entity);
    ASSERT_TRUE(B2_IS_NON_NULL(physics.bodyId));

    // Drive body movement directly in Box2D, then verify ECS transform sync.
    b2Body_SetLinearVelocity(physics.bodyId, {2.0f, -1.0f});

    const auto before = componentManager.getComponent<ecs::components::Transform>(entity);
    system.update(1.0, componentManager);
    const auto after = componentManager.getComponent<ecs::components::Transform>(entity);

    EXPECT_FLOAT_EQ(after.prevX, before.x);
    EXPECT_FLOAT_EQ(after.prevY, before.y);
    EXPECT_GT(after.x, before.x);
    EXPECT_LT(after.y, before.y);
}

TEST(PhysicsTransformCohabitationTests, PhysicsKeepsFinalTransformAuthorityWhenMovementAlsoRuns)
{
    engine::physics::BoxWorld world({0.0f, 0.0f});

    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();
    componentManager.registerComponent<ecs::components::PhysicsBody>();
    componentManager.registerComponent<ecs::components::Sprite>();

    constexpr ecs::EntityID entity = 102;
    componentManager.addComponent<ecs::components::Transform>(entity, makeTransform(0.0f, 0.0f));
    componentManager.addComponent<ecs::components::Velocity>(entity, ecs::components::Velocity{true, 10.0f, 0.0f});
    componentManager.addComponent<ecs::components::PhysicsBody>(entity, makePhysicsBody(true));

    ecs::systems::MovementSystem movement;
    movement.entities.insert(entity);

    ecs::systems::PhysicsSystem physics(&world);
    physics.entities.insert(entity);

    // Create body first.
    physics.update(1.0 / 60.0, componentManager);
    auto &physicsBody = componentManager.getComponent<ecs::components::PhysicsBody>(entity);
    ASSERT_TRUE(B2_IS_NON_NULL(physicsBody.bodyId));

    // Keep body static: if Movement changes Transform, Physics should overwrite it
    // from body transform in the same frame (current system order in Core).
    b2Body_SetLinearVelocity(physicsBody.bodyId, {0.0f, 0.0f});

    movement.update(1.0, componentManager);
    const auto movedByVelocity = componentManager.getComponent<ecs::components::Transform>(entity);
    EXPECT_GT(movedByVelocity.x, 0.0f);

    physics.update(1.0, componentManager);
    const auto finalTransform = componentManager.getComponent<ecs::components::Transform>(entity);

    EXPECT_NEAR(finalTransform.x, 0.0f, 0.0001f);
    EXPECT_NEAR(finalTransform.y, 0.0f, 0.0001f);
}
