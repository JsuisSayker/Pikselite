#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <gtest/gtest.h>
#include <tests/testHelpers.hpp>

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

    struct DummySystem1 : public ecs::ISystem
    {
        void update(double, engine::ComponentManager&) override {}
        void init() override {}
    };
    struct DummySystem2 : public ecs::ISystem
    {
        void update(double, engine::ComponentManager&) override {}
        void init() override {}
    };
    struct DummySystem3 : public ecs::ISystem
    {
        void update(double, engine::ComponentManager&) override {}
        void init() override {}
    };
} // namespace

TEST(EcsLoadTests, CreateMaxEntitiesWithTransform)
{
    TimedScope ts("CreateMaxEntitiesWithTransform", 5.0);

    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    for (ecs::EntityID id = 1; id <= ecs::MAX_ENTITIES; ++id)
    {
        ecs::Entity entity = entityManager.createEntity();
        EXPECT_EQ(entity.id, id);
        componentManager.addComponent<ecs::components::Transform>(id, makeTransform(0.0f, 0.0f));
    }

    for (ecs::EntityID id = 1; id <= ecs::MAX_ENTITIES; ++id)
    {
        EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(id));
        EXPECT_TRUE(entityManager.hasEntity(id));
    }
}

TEST(EcsLoadTests, EntityChurnRecyclesIds)
{
    engine::EntityManager entityManager;

    ecs::EntityID firstBatch[100];
    for (int i = 0; i < 100; ++i)
        firstBatch[i] = entityManager.createEntity().id;

    for (int i = 0; i < 50; ++i)
        entityManager.destroyEntity(ecs::Entity(firstBatch[i]));

    for (int i = 0; i < 50; ++i)
    {
        ecs::Entity e = entityManager.createEntity();
        EXPECT_GE(e.id, firstBatch[0]);
        EXPECT_LE(e.id, firstBatch[49]);
    }

    EXPECT_TRUE(entityManager.hasEntity(firstBatch[50]));
    EXPECT_TRUE(entityManager.hasEntity(firstBatch[99]));
}

TEST(EcsLoadTests, BulkAddRemoveComponents)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();

    std::vector<ecs::EntityID> ids;
    for (int i = 0; i < 100; ++i)
        ids.push_back(entityManager.createEntity().id);

    for (auto id : ids)
    {
        componentManager.addComponent<ecs::components::Transform>(id, makeTransform(1.0f, 2.0f));
        componentManager.addComponent<ecs::components::Velocity>(id, makeVelocity(3.0f, 4.0f));
    }

    for (auto id : ids)
    {
        EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(id));
        EXPECT_TRUE(componentManager.hasComponent<ecs::components::Velocity>(id));
    }

    for (auto id : ids)
        componentManager.removeComponent<ecs::components::Transform>(id);

    for (auto id : ids)
    {
        EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(id));
        EXPECT_TRUE(componentManager.hasComponent<ecs::components::Velocity>(id));
    }

    for (auto id : ids)
        componentManager.addComponent<ecs::components::Transform>(id, makeTransform(5.0f, 6.0f));

    for (auto id : ids)
        EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(id));
}

TEST(EcsLoadTests, ManySystemsTracked)
{
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;
    componentManager.registerComponent<ecs::components::Transform>();

    auto& sys1 = systemManager.addSystem<DummySystem1>();
    auto& sys2 = systemManager.addSystem<DummySystem2>();
    auto& sys3 = systemManager.addSystem<DummySystem3>();

    ecs::Signature sig;
    sig.set(componentManager.getComponentType<ecs::components::Transform>());
    systemManager.setSignature<DummySystem1>(sig);
    systemManager.setSignature<DummySystem2>(sig);
    systemManager.setSignature<DummySystem3>(sig);

    constexpr ecs::EntityID entity = 42;
    systemManager.entitySignatureChanged(entity, sig);

    EXPECT_EQ(sys1.entities.count(entity), 1u);
    EXPECT_EQ(sys2.entities.count(entity), 1u);
    EXPECT_EQ(sys3.entities.count(entity), 1u);

    ecs::Signature emptySig;
    systemManager.entitySignatureChanged(entity, emptySig);

    EXPECT_EQ(sys1.entities.count(entity), 0u);
    EXPECT_EQ(sys2.entities.count(entity), 0u);
    EXPECT_EQ(sys3.entities.count(entity), 0u);
}

TEST(EcsLoadTests, MultipleComponentTypesOnManyEntities)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();

    for (ecs::EntityID id = 1; id <= 500; ++id)
    {
        ecs::Entity entity = entityManager.createEntity();
        componentManager.addComponent<ecs::components::Transform>(id,
                                                                  makeTransform(static_cast<float>(id), 0.0f));
        componentManager.addComponent<ecs::components::Velocity>(id,
                                                                 makeVelocity(1.0f, static_cast<float>(id)));
    }

    for (ecs::EntityID id = 1; id <= 500; ++id)
    {
        const auto& t = componentManager.getComponent<ecs::components::Transform>(id);
        const auto& v = componentManager.getComponent<ecs::components::Velocity>(id);
        EXPECT_FLOAT_EQ(t.x, static_cast<float>(id));
        EXPECT_FLOAT_EQ(v.vy, static_cast<float>(id));
    }
}

TEST(EcsLoadTests, EntityIdRecycling)
{
    engine::EntityManager entityManager;

    ecs::EntityID first = entityManager.createEntity().id;
    entityManager.destroyEntity(ecs::Entity(first));

    ecs::Entity recycled = entityManager.createEntity();
    EXPECT_EQ(recycled.id, first);
}

TEST(EcsLoadTests, SystemUpdateWithManyEntities)
{
    struct TrackingSystem : public ecs::ISystem
    {
        int updateCount = 0;
        void update(double, engine::ComponentManager&) override { ++updateCount; }
        void init() override {}
    };

    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;
    componentManager.registerComponent<ecs::components::Transform>();

    auto& sys = systemManager.addSystem<TrackingSystem>();
    ecs::Signature sig;
    sig.set(componentManager.getComponentType<ecs::components::Transform>());
    systemManager.setSignature<TrackingSystem>(sig);

    for (ecs::EntityID id = 1; id <= 1000; ++id)
    {
        ecs::Signature entitySig;
        entitySig.set(componentManager.getComponentType<ecs::components::Transform>());
        systemManager.entitySignatureChanged(id, entitySig);
    }

    EXPECT_EQ(sys.entities.size(), 1000u);

    systemManager.update(1.0, componentManager);
    EXPECT_EQ(sys.updateCount, 1);

    for (ecs::EntityID id = 1; id <= 1000; ++id)
    {
        ecs::Signature emptySig;
        systemManager.entitySignatureChanged(id, emptySig);
    }

    EXPECT_EQ(sys.entities.size(), 0u);
}

TEST(EcsLoadTests, DestroyAllEntities)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    std::vector<ecs::Entity> entities;
    for (int i = 0; i < 100; ++i)
        entities.push_back(entityManager.createEntity());

    for (auto& e : entities)
    {
        componentManager.addComponent<ecs::components::Transform>(e.id, makeTransform(0.0f, 0.0f));
        entityManager.destroyEntity(e);
        componentManager.entityDestroyed(e.id);
    }

    for (auto& e : entities)
    {
        EXPECT_FALSE(entityManager.hasEntity(e.id));
        EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(e.id));
    }
}

TEST(EcsLoadTests, NearCapacityComponentArray)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    for (ecs::EntityID id = 1; id <= ecs::MAX_ENTITIES; ++id)
    {
        entityManager.createEntity();
        componentManager.addComponent<ecs::components::Transform>(id, makeTransform(0.0f, 0.0f));
    }

    EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(ecs::MAX_ENTITIES));

    componentManager.removeComponent<ecs::components::Transform>(ecs::MAX_ENTITIES);
    EXPECT_FALSE(componentManager.hasComponent<ecs::components::Transform>(ecs::MAX_ENTITIES));

    ecs::Entity newEntity = entityManager.createEntity();
    componentManager.addComponent<ecs::components::Transform>(newEntity.id, makeTransform(1.0f, 1.0f));
    EXPECT_TRUE(componentManager.hasComponent<ecs::components::Transform>(newEntity.id));
}
