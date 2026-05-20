-- Mini-game player controller — mouse-aimed water hose + full Lua API coverage.
--
-- Controls:
--   WASD / Arrows         move
--   LShift                sprint
--   Mouse                 aim
--   LMB (hold)            water hose — sprays water particles toward the cursor
--   RMB (click)           drop a sand puff at the cursor (single click, not stream)
--   E / Q                 spawn / despawn decoy entity
--   R                     toggle player sprite (remove/add component)
--   F                     reload the scene (Lua → EventBus → loadScene)
--   Space                 release camera (clear_camera_follow), re-grab on next move
--   Escape                quit

local PIXEL_SIZE = 10.0  -- must match the engine's PIXEL_SIZE

-- Hose tuning ----------------------------------------------------------------
local HOSE_SPEED          = 1.5   -- grid units per frame at the nozzle
local HOSE_PARTICLES_FRAME = 1     -- particles per frame while held
local HOSE_SPREAD          = 0.2  -- angular jitter (radians) — controls cone width
local HOSE_LIFETIME        = 10    -- frames before a droplet evaporates
local HOSE_OFFSET          = 2.4   -- grid units of muzzle offset from the player center

local move_speed = 130.0
local jump_high = 500.0
local spawn_key_was_down = false
local delete_key_was_down = false
local toggle_sprite_was_down = false
local reload_was_down = false
local space_was_down = false
local rmb_was_down = false
local decoy_id = nil
local last_facing_x = 1.0
local last_facing_y = 0.0
local player_half_size = 18.0
local spawn_padding = 8.0
local camera_released = false

local function set_velocity(vx, vy)
    if not has_component(entity.id, "velocity") then
        return
    end

    local vel = get_component(entity.id, "velocity")
    vel.vx = vx
    vel.vy = vy
    set_component(entity.id, "velocity", vel)

    entity.vx = vx
    entity.vy = vy
end

local function is_pressed_once(key, previous)
    local down = is_key_pressed(key)
    local once = down and (not previous)
    return down, once
end

-- Continuous water-hose spray. Aims from a point just in front of the player
-- toward the mouse cursor, then spreads a few particles per frame in a cone.
local function spray_particle_at_mouse(name)
    local mouse_world = get_mouse_world_position()
    if mouse_world == nil then
        return
    end

    -- Aim direction (world units) — normalize against player position.
    local dx = mouse_world.x - entity.x
    local dy = mouse_world.y - entity.y
    local dist = math.sqrt(dx * dx + dy * dy)
    if dist < 0.0001 then
        return
    end
    local nx = dx / dist
    local ny = dy / dist
    local base_angle = math.atan(ny, nx)

    -- Muzzle position: player center + a few grid cells along the aim vector.
    local muzzle_world_x = entity.x + nx * HOSE_OFFSET * PIXEL_SIZE
    local muzzle_world_y = entity.y + ny * HOSE_OFFSET * PIXEL_SIZE
    local muzzle_gx = math.floor(muzzle_world_x / PIXEL_SIZE)
    local muzzle_gy = math.floor(muzzle_world_y / PIXEL_SIZE)

    for i = 1, HOSE_PARTICLES_FRAME do
        local jitter = (math.random() - 0.5) * 2.0 * HOSE_SPREAD
        local angle = base_angle + jitter
        local speed = HOSE_SPEED * (0.85 + math.random() * 0.3)
        local vx = math.cos(angle) * speed
        local vy = math.sin(angle) * speed
        spawn_particle(muzzle_gx, muzzle_gy, name, vx, vy, HOSE_LIFETIME)
    end
end

function update(dt)
    local game_over = is_victory() or is_lose()

    -- Camera tracking — only when the run is still going and the player hasn't released it.
    if not game_over and not camera_released then
        follow_entity(entity.id)
    end

    if game_over then
        set_velocity(0.0, 0.0)
        -- Even after losing, allow F to reload so the player can retry.
        local reload_once
        reload_once, reload_was_down = is_pressed_once("f", reload_was_down)
        if reload_once then
            log("Player requested scene reload")
            reload_scene()
        end
        return
    end

    -- Movement intent ---------------------------------------------------------
    local vx = 0.0
    local vy = 0.0

    if is_key_pressed("a") or is_key_pressed("left") then
        vx = vx - move_speed
    end
    if is_key_pressed("d") or is_key_pressed("right") then
        vx = vx + move_speed 
    end
    if is_key_pressed("w") or is_key_pressed("up") then
        vy = vy + jump_high
    end
    if is_key_pressed("s") or is_key_pressed("down") then
        vy = vy - move_speed
    end

    if is_key_pressed("lshift") then
        vx = vx * 1.8
        vy = vy * 1.8
    end

    if vx ~= 0.0 or vy ~= 0.0 then
        if math.abs(vx) >= math.abs(vy) then
            last_facing_x = (vx >= 0.0) and 1.0 or -1.0
            last_facing_y = 0.0
        else
            last_facing_x = 0.0
            last_facing_y = (vy >= 0.0) and 1.0 or -1.0
        end
        if camera_released then
            camera_released = false
            follow_entity(entity.id)
            log("Camera re-attached to player")
        end
    end

    set_velocity(vx, vy)

    -- Water hose (LMB held — continuous spray) -------------------------------
    if is_mouse_pressed("left") then
        spray_particle_at_mouse("Water")
    end
    if is_mouse_pressed("right") then
        spray_particle_at_mouse("Fire")
    end

    -- Toggle player sprite (remove/add component) -----------------------------
    local toggle_once
    toggle_once, toggle_sprite_was_down = is_pressed_once("r", toggle_sprite_was_down)
    if toggle_once then
        if has_component(entity.id, "sprite") then
            remove_component(entity.id, "sprite")
            log("Player sprite removed")
        else
            add_component(entity.id, "sprite", {
                enabled = true,
                texturePath = "assets/dragon.png",
                width = 36.0,
                height = 36.0
            })
            log("Player sprite restored")
        end
    end

    -- Release / re-grab the camera (clear_camera_follow) ---------------------
    local space_once
    space_once, space_was_down = is_pressed_once("space", space_was_down)
    if space_once then
        clear_camera_follow()
        camera_released = true
        log("Camera released — move to re-attach")
    end

    -- Manual scene reload ----------------------------------------------------
    local reload_once
    reload_once, reload_was_down = is_pressed_once("f", reload_was_down)
    if reload_once then
        log("Player requested scene reload")
        reload_scene()
    end
end

function on_collision_enter(other_id)
    local other = get_entity(other_id)
    if other == nil then
        return
    end

    if other.name == "Hunter" then
        set_lose("Player touched by Hunter")
        return
    end

    if other.name == "Mine" or other.name == "TimedMine" then
        set_lose("Player hit a Mine")
        return
    end

    if other.name == "Crystal" then
        delete_entity(other_id)
        log("Crystal collected")

        local gate = get_entity_by_name("ExitGate")
        if gate ~= nil and has_component(gate.id, "sprite") then
            local s = get_component(gate.id, "sprite")
            s.texturePath = "assets/icon.bmp"
            s.width = 44.0
            s.height = 44.0
            set_component(gate.id, "sprite", s)
            log("Gate unlocked")
        end
        return
    end

    if other.name == "ExitGate" then
        local crystal = get_entity_by_name("Crystal")
        if crystal == nil then
            set_victory("Crystal delivered to gate")
        else
            log("Gate reached but Crystal still exists")
        end
        return
    end
end

function on_collision_exit(other_id)
    local other = get_entity(other_id)
    if other ~= nil then
        log("Player collision exit: " .. tostring(other.name))
    end
end

function on_victory(reason)
    log("Player victory: " .. tostring(reason))
end

function on_lose(reason)
    log("Player lose: " .. tostring(reason))
end
