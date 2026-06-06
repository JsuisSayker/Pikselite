-- Mini-game manager — sets up the level and drives the win/lose flow.
--
-- Attach this script to an invisible "GameManager" entity (Transform only, no sprite).
-- It exists to exercise scripting APIs that don't fit on the player or hunter:
--   - create_pixels (lava field, walls)
--   - get_entities (entity census log)
--   - load_scene / reload_scene (auto-restart, manual restart)
--   - clear_camera_follow on game-end
--   - on_victory / on_lose callbacks

local PIXEL_SIZE = 10.0
local initialized = false
local elapsed = 0.0
local last_entity_count_log = 0.0
local end_handled = false
local end_time = 0.0
local reload_delay = 3.0  -- seconds between game-end and auto-reload

function update(dt)
    elapsed = elapsed + dt

    if not initialized then
        initialized = true

        local list = get_entities()
        log("GameManager: initial entity count=" .. tostring(#list))
    end

    -- Periodic census so get_entities() is exercised at runtime too.
    if elapsed - last_entity_count_log > 4.0 then
        last_entity_count_log = elapsed
        local list = get_entities()
        log("GameManager: live entity count=" .. tostring(#list))
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
