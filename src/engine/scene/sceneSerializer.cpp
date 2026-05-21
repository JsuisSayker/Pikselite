#include <algorithm>
#include <cstddef>
#include <cstring>
#include <engine/ecs/components/physicsComponent.hpp>
#include <engine/ecs/components/scriptComponent.hpp>
#include <engine/ecs/components/spriteComponent.hpp>
#include <engine/ecs/components/transformComponent.hpp>
#include <engine/ecs/components/velocityComponent.hpp>
#include <engine/scene/sceneSerializer.hpp>
#include <fstream>
#include <limits>
#include <zlib.h>

namespace engine::scene
{
    namespace
    {
        constexpr uint32_t kSceneMagic = 0x5343534E; // 'SCSN'
        constexpr uint16_t kSceneVersion = 4;

        enum ComponentMask : uint32_t
        {
            MaskTransform = 1u << 0,
            MaskVelocity = 1u << 1,
            MaskSprite = 1u << 2,
            MaskPhysicsBody = 1u << 3,
            MaskScript = 1u << 4
        };

        struct Header
        {
            uint32_t magic = kSceneMagic;
            uint16_t version = kSceneVersion;
            uint16_t reserved = 0;
            uint32_t uncompressedSize = 0;
            uint32_t compressedSize = 0;
        };

        void writeBytes(std::vector<uint8_t>& out, const void* data, size_t size)
        {
            const auto* bytes = static_cast<const uint8_t*>(data);
            out.insert(out.end(), bytes, bytes + size);
        }

        void writeU8(std::vector<uint8_t>& out, uint8_t value)
        {
            writeBytes(out, &value, sizeof(value));
        }

        void writeU16(std::vector<uint8_t>& out, uint16_t value)
        {
            writeBytes(out, &value, sizeof(value));
        }

        void writeU32(std::vector<uint8_t>& out, uint32_t value)
        {
            writeBytes(out, &value, sizeof(value));
        }

        void writeI32(std::vector<uint8_t>& out, int32_t value)
        {
            writeBytes(out, &value, sizeof(value));
        }

        void writeF32(std::vector<uint8_t>& out, float value)
        {
            writeBytes(out, &value, sizeof(value));
        }

        void writeBool(std::vector<uint8_t>& out, bool value)
        {
            const uint8_t v = value ? 1u : 0u;
            writeU8(out, v);
        }

        void writeString(std::vector<uint8_t>& out, const std::string& value)
        {
            if (value.size() > std::numeric_limits<uint32_t>::max())
            {
                writeU32(out, 0);
                return;
            }
            const uint32_t size = static_cast<uint32_t>(value.size());
            writeU32(out, size);
            if (size > 0)
            {
                writeBytes(out, value.data(), size);
            }
        }

        bool readBytes(const std::vector<uint8_t>& data, size_t& offset, void* out, size_t size)
        {
            if (offset + size > data.size())
            {
                return false;
            }
            std::memcpy(out, data.data() + offset, size);
            offset += size;
            return true;
        }

        bool readU8(const std::vector<uint8_t>& data, size_t& offset, uint8_t& out)
        {
            return readBytes(data, offset, &out, sizeof(out));
        }

        bool readU16(const std::vector<uint8_t>& data, size_t& offset, uint16_t& out)
        {
            return readBytes(data, offset, &out, sizeof(out));
        }

        bool readU32(const std::vector<uint8_t>& data, size_t& offset, uint32_t& out)
        {
            return readBytes(data, offset, &out, sizeof(out));
        }

        bool readI32(const std::vector<uint8_t>& data, size_t& offset, int32_t& out)
        {
            return readBytes(data, offset, &out, sizeof(out));
        }

        bool readF32(const std::vector<uint8_t>& data, size_t& offset, float& out)
        {
            return readBytes(data, offset, &out, sizeof(out));
        }

        bool readBool(const std::vector<uint8_t>& data, size_t& offset, bool& out)
        {
            uint8_t value = 0;
            if (!readU8(data, offset, value))
            {
                return false;
            }
            out = (value != 0u);
            return true;
        }

        bool readString(const std::vector<uint8_t>& data, size_t& offset, std::string& out)
        {
            uint32_t size = 0;
            if (!readU32(data, offset, size))
            {
                return false;
            }
            if (offset + size > data.size())
            {
                return false;
            }
            out.assign(reinterpret_cast<const char*>(data.data() + offset), size);
            offset += size;
            return true;
        }

        uint32_t buildComponentMask(const Pixel::GameObject& go)
        {
            uint32_t mask = 0;
            if (go.hasComponent<ecs::components::Transform>())
            {
                mask |= MaskTransform;
            }
            if (go.hasComponent<ecs::components::Velocity>())
            {
                mask |= MaskVelocity;
            }
            if (go.hasComponent<ecs::components::Sprite>())
            {
                mask |= MaskSprite;
            }
            if (go.hasComponent<ecs::components::PhysicsBody>())
            {
                mask |= MaskPhysicsBody;
            }
            if (go.hasComponent<ecs::components::Script>())
            {
                mask |= MaskScript;
            }
            return mask;
        }

        void serializeComponentTransform(std::vector<uint8_t>& out,
                                         const ecs::components::Transform& t)
        {
            writeBool(out, t.enabled);
            writeF32(out, t.x);
            writeF32(out, t.y);
            writeF32(out, t.rotation);
            writeF32(out, t.scaleX);
            writeF32(out, t.scaleY);
        }

        void serializeComponentVelocity(std::vector<uint8_t>& out,
                                        const ecs::components::Velocity& v)
        {
            writeBool(out, v.enabled);
            writeF32(out, v.vx);
            writeF32(out, v.vy);
        }

        void serializeComponentSprite(std::vector<uint8_t>& out, const ecs::components::Sprite& s)
        {
            writeBool(out, s.enabled);
            writeString(out, s.texturePath);
            writeF32(out, s.width);
            writeF32(out, s.height);
            writeI32(out, s.layer);
        }

        void serializeComponentPhysicsBody(std::vector<uint8_t>& out,
                                           const ecs::components::PhysicsBody& p)
        {
            writeBool(out, p.enabled);
            writeU8(out, static_cast<uint8_t>(p.bodyType));
            writeBool(out, p.fixedRotation);
            writeF32(out, p.density);
            writeF32(out, p.friction);
            writeF32(out, p.restitution);
            const uint32_t count = static_cast<uint32_t>(p.triangles.size());
            writeU32(out, count);
            for (const auto& tri : p.triangles)
            {
                writeF32(out, tri.a.x);
                writeF32(out, tri.a.y);
                writeF32(out, tri.b.x);
                writeF32(out, tri.b.y);
                writeF32(out, tri.c.x);
                writeF32(out, tri.c.y);
            }
        }

        void serializeComponentScript(std::vector<uint8_t>& out, const ecs::components::Script& s)
        {
            writeBool(out, s.enabled);
            writeString(out, s.scriptPath);
        }

        bool deserializeComponentTransform(const std::vector<uint8_t>& data, size_t& offset,
                                           ecs::components::Transform& t)
        {
            if (!readBool(data, offset, t.enabled))
                return false;
            if (!readF32(data, offset, t.x))
                return false;
            if (!readF32(data, offset, t.y))
                return false;
            if (!readF32(data, offset, t.rotation))
                return false;
            if (!readF32(data, offset, t.scaleX))
                return false;
            if (!readF32(data, offset, t.scaleY))
                return false;
            t.prevX = t.x;
            t.prevY = t.y;
            return true;
        }

        bool deserializeComponentVelocity(const std::vector<uint8_t>& data, size_t& offset,
                                          ecs::components::Velocity& v)
        {
            if (!readBool(data, offset, v.enabled))
                return false;
            if (!readF32(data, offset, v.vx))
                return false;
            if (!readF32(data, offset, v.vy))
                return false;
            return true;
        }

        bool deserializeComponentSprite(const std::vector<uint8_t>& data, size_t& offset,
                                        ecs::components::Sprite& s)
        {
            if (!readBool(data, offset, s.enabled))
                return false;
            if (!readString(data, offset, s.texturePath))
                return false;
            if (!readF32(data, offset, s.width))
                return false;
            if (!readF32(data, offset, s.height))
                return false;
            if (!readI32(data, offset, s.layer))
                return false;
            s.textureID = 0;
            s.loaded = false;
            return true;
        }

        bool deserializeComponentPhysicsBody(const std::vector<uint8_t>& data, size_t& offset,
                                             ecs::components::PhysicsBody& p)
        {
            uint8_t bodyType = 0;
            if (!readBool(data, offset, p.enabled))
                return false;
            if (!readU8(data, offset, bodyType))
                return false;
            if (!readBool(data, offset, p.fixedRotation))
                return false;
            if (!readF32(data, offset, p.density))
                return false;
            if (!readF32(data, offset, p.friction))
                return false;
            if (!readF32(data, offset, p.restitution))
                return false;
            uint32_t count = 0;
            if (!readU32(data, offset, count))
                return false;
            p.bodyType = static_cast<b2BodyType>(bodyType);
            p.bodyId = b2_nullBodyId;
            p.triangles.clear();
            p.triangles.reserve(count);
            for (uint32_t i = 0; i < count; ++i)
            {
                ecs::components::PhysicsTriangle tri;
                if (!readF32(data, offset, tri.a.x))
                    return false;
                if (!readF32(data, offset, tri.a.y))
                    return false;
                if (!readF32(data, offset, tri.b.x))
                    return false;
                if (!readF32(data, offset, tri.b.y))
                    return false;
                if (!readF32(data, offset, tri.c.x))
                    return false;
                if (!readF32(data, offset, tri.c.y))
                    return false;
                p.triangles.push_back(tri);
            }
            return true;
        }

        bool deserializeComponentScript(const std::vector<uint8_t>& data, size_t& offset,
                                        ecs::components::Script& s)
        {
            if (!readBool(data, offset, s.enabled))
                return false;
            if (!readString(data, offset, s.scriptPath))
                return false;
            return true;
        }

        void serializeGameObject(std::vector<uint8_t>& out, const Pixel::GameObject& go)
        {
            writeU32(out, go.id);
            writeBool(out, go.isActive);
            writeString(out, go.name);
            writeString(out, go.sourceDatPath);

            const uint32_t componentMask = buildComponentMask(go);
            writeU32(out, componentMask);

            if (componentMask & MaskTransform)
            {
                const auto* t = go.getComponent<ecs::components::Transform>();
                serializeComponentTransform(out, t ? *t : ecs::components::Transform{});
            }
            if (componentMask & MaskVelocity)
            {
                const auto* v = go.getComponent<ecs::components::Velocity>();
                serializeComponentVelocity(out, v ? *v : ecs::components::Velocity{});
            }
            if (componentMask & MaskSprite)
            {
                const auto* s = go.getComponent<ecs::components::Sprite>();
                serializeComponentSprite(out, s ? *s : ecs::components::Sprite{});
            }
            if (componentMask & MaskPhysicsBody)
            {
                const auto* p = go.getComponent<ecs::components::PhysicsBody>();
                serializeComponentPhysicsBody(out, p ? *p : ecs::components::PhysicsBody{});
            }
            if (componentMask & MaskScript)
            {
                const auto* s = go.getComponent<ecs::components::Script>();
                serializeComponentScript(out, s ? *s : ecs::components::Script{});
            }

            const uint32_t pixelCount =
                static_cast<uint32_t>(std::min(go.pixels.size(), go.pixelLocalCoords.size()));
            writeU32(out, pixelCount);
            for (uint32_t i = 0; i < pixelCount; ++i)
            {
                const auto& local = go.pixelLocalCoords[i];
                const auto& pixel = go.pixels[i];
                writeI32(out, local.x);
                writeI32(out, local.y);
                writeU16(out, static_cast<uint16_t>(pixel.type));
                writeU8(out, pixel.colorIndex);
                writeU8(out, pixel.burnTimer);
                writeBool(out, pixel.isBurning);
            }
        }

        bool deserializeGameObject(const std::vector<uint8_t>& data, size_t& offset,
                                   Pixel::GameObject& outGo)
        {
            if (!readU32(data, offset, outGo.id))
                return false;
            if (!readBool(data, offset, outGo.isActive))
                return false;
            if (!readString(data, offset, outGo.name))
                return false;
            if (!readString(data, offset, outGo.sourceDatPath))
                return false;

            uint32_t componentMask = 0;
            if (!readU32(data, offset, componentMask))
                return false;

            outGo.components.clear();

            if (componentMask & MaskTransform)
            {
                ecs::components::Transform t;
                if (!deserializeComponentTransform(data, offset, t))
                    return false;
                outGo.addComponent(t);
            }
            if (componentMask & MaskVelocity)
            {
                ecs::components::Velocity v;
                if (!deserializeComponentVelocity(data, offset, v))
                    return false;
                outGo.addComponent(v);
            }
            if (componentMask & MaskSprite)
            {
                ecs::components::Sprite s;
                if (!deserializeComponentSprite(data, offset, s))
                    return false;
                outGo.addComponent(s);
            }
            if (componentMask & MaskPhysicsBody)
            {
                ecs::components::PhysicsBody p;
                if (!deserializeComponentPhysicsBody(data, offset, p))
                    return false;
                outGo.addComponent(p);
            }
            if (componentMask & MaskScript)
            {
                ecs::components::Script s;
                if (!deserializeComponentScript(data, offset, s))
                    return false;
                outGo.addComponent(s);
            }

            uint32_t pixelCount = 0;
            if (!readU32(data, offset, pixelCount))
                return false;
            outGo.pixels.clear();
            outGo.pixelLocalCoords.clear();
            outGo.pixels.reserve(pixelCount);
            outGo.pixelLocalCoords.reserve(pixelCount);

            for (uint32_t i = 0; i < pixelCount; ++i)
            {
                int32_t localX = 0;
                int32_t localY = 0;
                uint16_t type = 0;
                uint8_t colorIndex = 0;
                uint8_t burnTimer = 0;
                bool isBurning = false;
                if (!readI32(data, offset, localX))
                    return false;
                if (!readI32(data, offset, localY))
                    return false;
                if (!readU16(data, offset, type))
                    return false;
                if (!readU8(data, offset, colorIndex))
                    return false;
                if (!readU8(data, offset, burnTimer))
                    return false;
                if (!readBool(data, offset, isBurning))
                    return false;

                outGo.pixelLocalCoords.push_back({localX, localY});
                Element::Pixel px;
                px.type = static_cast<Element::ElementType>(type);
                px.colorIndex = colorIndex;
                px.burnTimer = burnTimer;
                px.isBurning = isBurning;
                outGo.pixels.push_back(px);
            }

            return true;
        }

        bool compressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output)
        {
            if (input.empty())
            {
                output.clear();
                return true;
            }

            uLongf destLen = compressBound(static_cast<uLong>(input.size()));
            output.resize(destLen);

            const int result = compress2(output.data(), &destLen, input.data(),
                                         static_cast<uLong>(input.size()), Z_BEST_COMPRESSION);
            if (result != Z_OK)
            {
                return false;
            }

            output.resize(destLen);
            return true;
        }

        bool decompressBuffer(const std::vector<uint8_t>& input, size_t expectedSize,
                              std::vector<uint8_t>& output)
        {
            output.resize(expectedSize);
            if (expectedSize == 0)
            {
                return true;
            }

            uLongf destLen = static_cast<uLongf>(expectedSize);
            const int result =
                uncompress(output.data(), &destLen, input.data(), static_cast<uLong>(input.size()));
            if (result != Z_OK || destLen != expectedSize)
            {
                return false;
            }

            return true;
        }
    } // namespace

    bool saveSceneToFile(const std::string& filename, const SceneData& data)
    {
        std::vector<uint8_t> raw;
        raw.reserve(1024);

        writeU32(raw, data.nextGameObjectId);
        writeF32(raw, data.cameraX);
        writeF32(raw, data.cameraY);
        writeF32(raw, data.cameraZoom);
        const uint32_t count = static_cast<uint32_t>(data.gameObjects.size());
        writeU32(raw, count);

        for (const auto& go : data.gameObjects)
        {
            serializeGameObject(raw, go);
        }

        std::vector<uint8_t> compressed;
        if (!compressBuffer(raw, compressed))
        {
            return false;
        }

        Header header;
        header.uncompressedSize = static_cast<uint32_t>(raw.size());
        header.compressedSize = static_cast<uint32_t>(compressed.size());

        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile)
        {
            return false;
        }

        outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
        if (!compressed.empty())
        {
            outFile.write(reinterpret_cast<const char*>(compressed.data()),
                          static_cast<std::streamsize>(compressed.size()));
        }

        return static_cast<bool>(outFile);
    }

    bool loadSceneFromFile(const std::string& filename, SceneData& outData)
    {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile)
        {
            return false;
        }

        Header header;
        inFile.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!inFile || header.magic != kSceneMagic || header.version < 3 ||
            header.version > kSceneVersion)
        {
            return false;
        }

        std::vector<uint8_t> compressed(header.compressedSize);
        if (header.compressedSize > 0)
        {
            inFile.read(reinterpret_cast<char*>(compressed.data()),
                        static_cast<std::streamsize>(compressed.size()));
            if (!inFile)
            {
                return false;
            }
        }

        std::vector<uint8_t> raw;
        if (!decompressBuffer(compressed, header.uncompressedSize, raw))
        {
            return false;
        }

        size_t offset = 0;
        SceneData temp;
        if (!readU32(raw, offset, temp.nextGameObjectId))
            return false;

        if (header.version >= 4)
        {
            if (!readF32(raw, offset, temp.cameraX))
                return false;
            if (!readF32(raw, offset, temp.cameraY))
                return false;
            if (!readF32(raw, offset, temp.cameraZoom))
                return false;
        }

        uint32_t count = 0;
        if (!readU32(raw, offset, count))
            return false;

        temp.gameObjects.clear();
        temp.gameObjects.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            Pixel::GameObject go;
            if (!deserializeGameObject(raw, offset, go))
            {
                return false;
            }
            temp.gameObjects.push_back(std::move(go));
        }

        outData = std::move(temp);
        return true;
    }
} // namespace engine::scene
