#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/managers/componentManager.hpp>
#include <engine/managers/entityManager.hpp>
#include <engine/managers/systemManager.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
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

    template <std::size_t N> struct StressComp
    {
        int data = static_cast<int>(N);
    };

    struct DummySystemForStress : public ecs::ISystem
    {
        void update(double, engine::ComponentManager&) override {}
        void init() override {}
    };
} // namespace

TEST(EcsStressTests, ComponentTypeAtExactLimit)
{
    engine::ComponentManager cm;

    for (std::size_t i = 0; i < ecs::MAX_COMPONENT_TYPES; ++i)
    {
        switch (i)
        {
            case 0:
                cm.registerComponent<StressComp<0>>();
                break;
            case 1:
                cm.registerComponent<StressComp<1>>();
                break;
            case 2:
                cm.registerComponent<StressComp<2>>();
                break;
            case 3:
                cm.registerComponent<StressComp<3>>();
                break;
            case 4:
                cm.registerComponent<StressComp<4>>();
                break;
            case 5:
                cm.registerComponent<StressComp<5>>();
                break;
            case 6:
                cm.registerComponent<StressComp<6>>();
                break;
            case 7:
                cm.registerComponent<StressComp<7>>();
                break;
            case 8:
                cm.registerComponent<StressComp<8>>();
                break;
            case 9:
                cm.registerComponent<StressComp<9>>();
                break;
            case 10:
                cm.registerComponent<StressComp<10>>();
                break;
            case 11:
                cm.registerComponent<StressComp<11>>();
                break;
            case 12:
                cm.registerComponent<StressComp<12>>();
                break;
            case 13:
                cm.registerComponent<StressComp<13>>();
                break;
            case 14:
                cm.registerComponent<StressComp<14>>();
                break;
            case 15:
                cm.registerComponent<StressComp<15>>();
                break;
            case 16:
                cm.registerComponent<StressComp<16>>();
                break;
            case 17:
                cm.registerComponent<StressComp<17>>();
                break;
            case 18:
                cm.registerComponent<StressComp<18>>();
                break;
            case 19:
                cm.registerComponent<StressComp<19>>();
                break;
            case 20:
                cm.registerComponent<StressComp<20>>();
                break;
            case 21:
                cm.registerComponent<StressComp<21>>();
                break;
            case 22:
                cm.registerComponent<StressComp<22>>();
                break;
            case 23:
                cm.registerComponent<StressComp<23>>();
                break;
            case 24:
                cm.registerComponent<StressComp<24>>();
                break;
            case 25:
                cm.registerComponent<StressComp<25>>();
                break;
            case 26:
                cm.registerComponent<StressComp<26>>();
                break;
            case 27:
                cm.registerComponent<StressComp<27>>();
                break;
            case 28:
                cm.registerComponent<StressComp<28>>();
                break;
            case 29:
                cm.registerComponent<StressComp<29>>();
                break;
            case 30:
                cm.registerComponent<StressComp<30>>();
                break;
            case 31:
                cm.registerComponent<StressComp<31>>();
                break;
        }
    }

    EXPECT_THROW(cm.registerComponent<StressComp<32>>(), std::runtime_error);
}

TEST(EcsStressTests, ComponentArrayFullOverflow)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    for (ecs::EntityID id = 1; id <= ecs::MAX_ENTITIES; ++id)
        componentManager.addComponent<ecs::components::Transform>(id, makeTransform(0.0f, 0.0f));

    EXPECT_THROW(componentManager.addComponent<ecs::components::Transform>(
                     ecs::MAX_ENTITIES + 1, makeTransform(0.0f, 0.0f)),
                 std::runtime_error);
}

TEST(EcsStressTests, SignatureBitOverflow)
{
    ecs::Signature sig;
    EXPECT_THROW(sig.set(ecs::MAX_COMPONENT_TYPES), std::out_of_range);
}

TEST(EcsStressTests, EntityManagerFreeListStress)
{
    engine::EntityManager entityManager;

    for (int cycle = 0; cycle < 100; ++cycle)
    {
        std::vector<ecs::Entity> batch;
        for (int i = 0; i < 50; ++i)
            batch.push_back(entityManager.createEntity());

        for (auto& e : batch)
            entityManager.destroyEntity(e);
    }
}

TEST(EcsStressTests, EmptyManagersDoNotCrash)
{
    engine::EntityManager entityManager;
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;

    EXPECT_FALSE(entityManager.hasEntity(1));
    EXPECT_NO_THROW(systemManager.update(1.0, componentManager));
    EXPECT_NO_THROW(systemManager.entityDestroyed(1));
    EXPECT_NO_THROW(componentManager.entityDestroyed(1));
}

TEST(EcsStressTests, TypeMismatchThrows)
{
    engine::ComponentManager componentManager;
    componentManager.registerComponent<ecs::components::Transform>();

    EXPECT_THROW(componentManager.getComponentType<ecs::components::Velocity>(),
                 std::runtime_error);
}

TEST(EcsStressTests, RapidSignatureChanges)
{
    engine::ComponentManager componentManager;
    engine::SystemManager systemManager;
    componentManager.registerComponent<ecs::components::Transform>();
    componentManager.registerComponent<ecs::components::Velocity>();

    auto& sys = systemManager.addSystem<DummySystemForStress>();
    ecs::Signature sig;
    sig.set(componentManager.getComponentType<ecs::components::Transform>());
    systemManager.setSignature<DummySystemForStress>(sig);

    for (int i = 0; i < 500; ++i)
    {
        ecs::Signature withTransform;
        withTransform.set(componentManager.getComponentType<ecs::components::Transform>());
        systemManager.entitySignatureChanged(static_cast<ecs::EntityID>(i), withTransform);

        ecs::Signature withVelocity;
        withVelocity.set(componentManager.getComponentType<ecs::components::Velocity>());
        systemManager.entitySignatureChanged(static_cast<ecs::EntityID>(i), withVelocity);

        ecs::Signature empty;
        systemManager.entitySignatureChanged(static_cast<ecs::EntityID>(i), empty);
    }
}
