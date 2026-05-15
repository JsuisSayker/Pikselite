-- Mini-game manager — sets up the level and drives the win/lose flow.
--
-- Attach this script to an invisible "GameManager" entity (Transform only, no sprite).
-- It exists to exercise scripting APIs that don't fit on the player or hunter:
--   - create_pixels (lava field, walls)
--   - get_entities (entity census log)
--   - create_entity / delete_entity (timed mine spawn-then-despawn)
--   - load_scene / reload_scene (auto-restart, manual restart)
--   - clear_camera_follow on game-end
--   - on_victory / on_lose callbacks

local PIXEL_SIZE = 10.0
local initialized = false
local mine_id = nil
local elapsed = 0.0
local last_entity_count_log = 0.0
local end_handled = false
local end_time = 0.0
local reload_delay = 3.0  -- seconds between game-end and auto-reload

-- Helper: spawn a rectangular blob of `material` in grid coords [gx0..gx1] × [gy0..gy1].
local function fill_rect(gx0, gy0, gx1, gy1, material)
    local batch = {}
    for y = gy0, gy1 do
        for x = gx0, gx1 do
            batch[#batch + 1] = { x = x, y = y, type = material }
        end
    end
    create_pixels(batch)
end

local function spawn_level_pixels()
    -- Two lava puddles for the player to extinguish with thrown water.
    fill_rect(40, 8, 48, 10, "Lava")
    fill_rect(70, 8, 78, 11, "Lava")

    -- A stone wall the player can build sand alternatives against.
    fill_rect(20, 8, 22, 14, "Stone")
    fill_rect(90, 8, 92, 14, "Stone")

    -- A dirt floor under the lava so the simulation has something to settle on.
    fill_rect(35, 6, 85, 7, "Dirt")
end

function update(dt)
    elapsed = elapsed + dt

    if not initialized then
        initialized = true

        local list = get_entities()
        log("GameManager: initial entity count=" .. tostring(#list))

        spawn_level_pixels()

        -- Spawn a temporary mine to demonstrate dynamic entity creation/deletion.
        mine_id = create_entity({
            name = "TimedMine",
            transform = { x = 420.0, y = 250.0, rotation = 0.0, scaleX = 1.0, scaleY = 1.0 },
            sprite  = { texturePath = "assets/icon.bmp", width = 14.0, height = 14.0 },
            physics = {
                bodyType = "static",
                fixedRotation = true,
                density = 1.0,
                friction = 0.0,
                restitution = 0.0
            }
        })
        log("GameManager: TimedMine id=" .. tostring(mine_id))
    end

    -- Periodic census so get_entities() is exercised at runtime too.
    if elapsed - last_entity_count_log > 4.0 then
        last_entity_count_log = elapsed
        local list = get_entities()
        log("GameManager: live entity count=" .. tostring(#list))
    end

    -- Despawn the timed mine after a while.
    if mine_id ~= nil and elapsed > 12.0 then
        if delete_entity(mine_id) then
            log("GameManager: TimedMine despawned")
        end
        mine_id = nil
    end

    -- Game-end flow: release the camera, then auto-reload after a delay.
    if not end_handled and (is_victory() or is_lose()) then
        end_handled = true
        end_time = elapsed
        clear_camera_follow()
        if is_victory() then
            log("GameManager: VICTORY — scene will reload in " .. tostring(reload_delay) .. "s")
        else
            log("GameManager: LOSE — scene will reload in " .. tostring(reload_delay) .. "s")
        end
    end

    if end_handled and (elapsed - end_time) >= reload_delay then
        log("GameManager: auto-reloading scene")
        reload_scene()
    end
end

function on_victory(reason)
    log("GameManager victory: " .. tostring(reason))
end

function on_lose(reason)
    log("GameManager lose: " .. tostring(reason))
end
