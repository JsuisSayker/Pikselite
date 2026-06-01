#include <box2d/box2d.h>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/scene/sceneSerializer.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace
{
    std::string tempScenePath()
    {
        return (std::filesystem::temp_directory_path() / "pikselite_test_scene.scene").string();
    }

    Pixel::GameObject makeGameObject(Pixel::GameObjectID id, const std::string& name)
    {
        Pixel::GameObject go;
        go.id = id;
        go.name = name;
        go.isActive = true;
        return go;
    }

    Pixel::GameObject makeGameObjectWithAllComponents(Pixel::GameObjectID id)
    {
        Pixel::GameObject go;
        go.id = id;
        go.name = "FullObject";
        go.isActive = true;

        ecs::components::Transform t{};
        t.enabled = true;
        t.x = 10.5f;
        t.y = 20.3f;
        t.rotation = 1.57f;
        t.scaleX = 2.0f;
        t.scaleY = 2.0f;
        go.addComponent(t);

        ecs::components::Velocity v{};
        v.enabled = true;
        v.vx = 3.0f;
        v.vy = -1.5f;
        go.addComponent(v);

        ecs::components::Sprite s{};
        s.enabled = true;
        s.texturePath = "assets/hero.png";
        s.width = 32.0f;
        s.height = 48.0f;
        go.addComponent(s);

        ecs::components::PhysicsBody p{};
        p.enabled = true;
        p.bodyType = b2_dynamicBody;
        p.fixedRotation = false;
        p.density = 2.0f;
        p.friction = 0.5f;
        p.restitution = 0.2f;
        ecs::components::PhysicsTriangle tri;
        tri.a = {0.0f, 0.0f};
        tri.b = {16.0f, 0.0f};
        tri.c = {8.0f, 16.0f};
        p.triangles.push_back(tri);
        go.addComponent(p);

        ecs::components::Script sc{};
        sc.enabled = true;
        sc.scriptPath = "scripts/enemy.lua";
        go.addComponent(sc);

        Element::Pixel px;
        px.type = Element::SAND;
        px.colorIndex = 2;
        px.burnTimer = 0;
        px.isBurning = false;
        go.pixels.push_back(px);
        go.pixelLocalCoords.push_back({0, 0});

        return go;
    }

    bool writeRawFile(const std::string& path, const std::vector<uint8_t>& data)
    {
        std::ofstream out(path, std::ios::binary);
        if (!out)
            return false;
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
        return out.good();
    }
} // namespace

TEST(SceneSerializationTests, NonExistentFile)
{
    engine::scene::SceneData data;
    EXPECT_FALSE(engine::scene::loadSceneFromFile("nonexistent_12345.scene", data));
}

TEST(SceneSerializationTests, CorruptMagicNumber)
{
    // Write a file with wrong magic
    std::vector<uint8_t> badData = {0x00, 0x00, 0x00, 0x00,  // bad magic
                                    0x03, 0x00,              // version 3
                                    0x00, 0x00,              // reserved
                                    0x00, 0x00, 0x00, 0x00,  // uncompressedSize
                                    0x00, 0x00, 0x00, 0x00}; // compressedSize
    const auto path = tempScenePath();
    ASSERT_TRUE(writeRawFile(path, badData));

    engine::scene::SceneData data;
    EXPECT_FALSE(engine::scene::loadSceneFromFile(path, data));
    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, WrongVersion)
{
    // Write a file with wrong version number
    std::vector<uint8_t> badVersion = {0x4E, 0x53, 0x43, 0x53,  // 'SCSN' magic
                                       0xFF, 0x00,              // version 255 (wrong)
                                       0x00, 0x00,              // reserved
                                       0x00, 0x00, 0x00, 0x00,  // uncompressedSize
                                       0x00, 0x00, 0x00, 0x00}; // compressedSize
    const auto path = tempScenePath();
    ASSERT_TRUE(writeRawFile(path, badVersion));

    engine::scene::SceneData data;
    EXPECT_FALSE(engine::scene::loadSceneFromFile(path, data));
    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, TruncatedFile)
{
    // Write a valid header but truncated body
    std::vector<uint8_t> header = {0x4E, 0x53, 0x43, 0x53,  // 'SCSN'
                                   0x03, 0x00,              // version 3
                                   0x00, 0x00,              // reserved
                                   0xFF, 0x00, 0x00, 0x00,  // uncompressedSize = 255
                                   0x10, 0x00, 0x00, 0x00}; // compressedSize = 16
    // Only write 4 bytes of compressed data instead of 16
    std::vector<uint8_t> truncated = header;
    truncated.insert(truncated.end(), {0x01, 0x02, 0x03, 0x04});

    const auto path = tempScenePath();
    ASSERT_TRUE(writeRawFile(path, truncated));

    engine::scene::SceneData data;
    EXPECT_FALSE(engine::scene::loadSceneFromFile(path, data));
    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, EmptySceneRoundTrip)
{
    engine::scene::SceneData saved;
    saved.nextGameObjectId = 1;
    saved.gameObjects.clear();

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    EXPECT_EQ(loaded.nextGameObjectId, saved.nextGameObjectId);
    EXPECT_TRUE(loaded.gameObjects.empty());

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, SingleGameObjectRoundTrip)
{
    engine::scene::SceneData saved;
    saved.nextGameObjectId = 42;
    saved.gameObjects.push_back(makeGameObject(1, "TestObject"));

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    EXPECT_EQ(loaded.nextGameObjectId, saved.nextGameObjectId);
    ASSERT_EQ(loaded.gameObjects.size(), 1u);
    EXPECT_EQ(loaded.gameObjects[0].id, 1u);
    EXPECT_EQ(loaded.gameObjects[0].name, "TestObject");

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, AllComponentTypesRoundTrip)
{
    engine::scene::SceneData saved;
    saved.nextGameObjectId = 100;
    saved.gameObjects.push_back(makeGameObjectWithAllComponents(7));

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    ASSERT_EQ(loaded.gameObjects.size(), 1u);
    const auto& go = loaded.gameObjects[0];
    EXPECT_EQ(go.id, 7u);
    EXPECT_EQ(go.name, "FullObject");

    ASSERT_TRUE(go.hasComponent<ecs::components::Transform>());
    const auto* t = go.getComponent<ecs::components::Transform>();
    EXPECT_FLOAT_EQ(t->x, 10.5f);
    EXPECT_FLOAT_EQ(t->y, 20.3f);
    EXPECT_FLOAT_EQ(t->rotation, 1.57f);
    EXPECT_FLOAT_EQ(t->scaleX, 2.0f);

    ASSERT_TRUE(go.hasComponent<ecs::components::Velocity>());
    const auto* v = go.getComponent<ecs::components::Velocity>();
    EXPECT_FLOAT_EQ(v->vx, 3.0f);
    EXPECT_FLOAT_EQ(v->vy, -1.5f);

    ASSERT_TRUE(go.hasComponent<ecs::components::Sprite>());
    const auto* s = go.getComponent<ecs::components::Sprite>();
    EXPECT_EQ(s->texturePath, "assets/hero.png");
    EXPECT_FLOAT_EQ(s->width, 32.0f);
    EXPECT_FLOAT_EQ(s->height, 48.0f);

    ASSERT_TRUE(go.hasComponent<ecs::components::PhysicsBody>());
    const auto* p = go.getComponent<ecs::components::PhysicsBody>();
    EXPECT_FLOAT_EQ(p->density, 2.0f);
    EXPECT_FLOAT_EQ(p->friction, 0.5f);
    ASSERT_EQ(p->triangles.size(), 1u);
    EXPECT_FLOAT_EQ(p->triangles[0].a.x, 0.0f);
    EXPECT_FLOAT_EQ(p->triangles[0].b.x, 16.0f);
    EXPECT_FLOAT_EQ(p->triangles[0].c.y, 16.0f);

    ASSERT_TRUE(go.hasComponent<ecs::components::Script>());
    const auto* sc = go.getComponent<ecs::components::Script>();
    EXPECT_EQ(sc->scriptPath, "scripts/enemy.lua");

    ASSERT_EQ(go.pixels.size(), 1u);
    EXPECT_EQ(go.pixels[0].type, Element::SAND);
    EXPECT_EQ(go.pixelLocalCoords[0].x, 0);
    EXPECT_EQ(go.pixelLocalCoords[0].y, 0);

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, MultipleGameObjectsRoundTrip)
{
    engine::scene::SceneData saved;
    saved.nextGameObjectId = 50;
    saved.gameObjects.push_back(makeGameObject(1, "First"));
    saved.gameObjects.push_back(makeGameObject(2, "Second"));
    saved.gameObjects.push_back(makeGameObject(3, "Third"));

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    ASSERT_EQ(loaded.gameObjects.size(), 3u);
    EXPECT_EQ(loaded.gameObjects[0].name, "First");
    EXPECT_EQ(loaded.gameObjects[1].name, "Second");
    EXPECT_EQ(loaded.gameObjects[2].name, "Third");

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, LargeTriangleCount)
{
    Pixel::GameObject go;
    go.id = 1;
    go.name = "ManyTriangles";
    go.isActive = true;

    ecs::components::PhysicsBody p;
    p.enabled = true;
    p.bodyType = b2_staticBody;
    for (int i = 0; i < 100; ++i)
    {
        ecs::components::PhysicsTriangle tri;
        tri.a = {static_cast<float>(i), 0.0f};
        tri.b = {static_cast<float>(i + 1), 0.0f};
        tri.c = {static_cast<float>(i), 1.0f};
        p.triangles.push_back(tri);
    }
    go.addComponent(p);

    engine::scene::SceneData saved;
    saved.gameObjects.push_back(go);

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    ASSERT_EQ(loaded.gameObjects.size(), 1u);
    ASSERT_TRUE(loaded.gameObjects[0].hasComponent<ecs::components::PhysicsBody>());
    const auto* loadedP = loaded.gameObjects[0].getComponent<ecs::components::PhysicsBody>();
    ASSERT_EQ(loadedP->triangles.size(), 100u);
    EXPECT_FLOAT_EQ(loadedP->triangles[99].a.x, 99.0f);

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, MaxStringLengths)
{
    Pixel::GameObject go;
    go.id = 1;
    go.name = std::string(1024, 'A');
    go.isActive = true;

    ecs::components::Sprite s;
    s.texturePath = std::string(2048, 'B');
    go.addComponent(s);

    ecs::components::Script sc;
    sc.scriptPath = std::string(1024, 'C');
    go.addComponent(sc);

    engine::scene::SceneData saved;
    saved.gameObjects.push_back(go);
    saved.nextGameObjectId = 2;

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    ASSERT_EQ(loaded.gameObjects.size(), 1u);
    EXPECT_EQ(loaded.gameObjects[0].name, std::string(1024, 'A'));
    ASSERT_TRUE(loaded.gameObjects[0].hasComponent<ecs::components::Sprite>());
    EXPECT_EQ(loaded.gameObjects[0].getComponent<ecs::components::Sprite>()->texturePath,
              std::string(2048, 'B'));

    std::filesystem::remove(path);
}

TEST(SceneSerializationTests, NegativeFloatValues)
{
    Pixel::GameObject go;
    go.id = 1;
    go.name = "NegativeCoords";
    go.isActive = true;

    ecs::components::Transform t;
    t.enabled = true;
    t.x = -100.5f;
    t.y = -200.75f;
    t.rotation = -3.14f;
    t.scaleX = -1.0f;
    t.scaleY = 1.0f;
    go.addComponent(t);

    engine::scene::SceneData saved;
    saved.gameObjects.push_back(go);

    const auto path = tempScenePath();
    EXPECT_TRUE(engine::scene::saveSceneToFile(path, saved));

    engine::scene::SceneData loaded;
    EXPECT_TRUE(engine::scene::loadSceneFromFile(path, loaded));
    ASSERT_EQ(loaded.gameObjects.size(), 1u);
    ASSERT_TRUE(loaded.gameObjects[0].hasComponent<ecs::components::Transform>());
    const auto* loadedT = loaded.gameObjects[0].getComponent<ecs::components::Transform>();
    EXPECT_FLOAT_EQ(loadedT->x, -100.5f);
    EXPECT_FLOAT_EQ(loadedT->y, -200.75f);
    EXPECT_FLOAT_EQ(loadedT->rotation, -3.14f);

    std::filesystem::remove(path);
}
