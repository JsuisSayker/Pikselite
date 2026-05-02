-- Demo mini-game player controller for Lua API validation.

local move_speed = 130.0
local spawn_key_was_down = false
local delete_key_was_down = false
local toggle_sprite_was_down = false
local spawn_mine_was_down = false
local mouse_click_was_down = false
local decoy_id = nil
local last_facing_x = 1.0
local last_facing_y = 0.0
local player_half_size = 18.0
local spawn_padding = 8.0

local function set_velocity(vx, vy)
    if not has_component(entity.id, "velocity") then
        return
    end

    local vel = get_component(entity.id, "velocity")
    vel.vx = vx
    vel.vy = vy
    set_component(entity.id, "velocity", vel)

    -- Keep the frame-local table coherent with set_component writes.
    entity.vx = vx
    entity.vy = vy
end

local function is_pressed_once(key, previous)
    local down = is_key_pressed(key)
    local once = down and (not previous)
    return down, once
end

function update(dt)
    follow_entity(entity.id)

    if is_victory() or is_lose() then
        set_velocity(0.0, 0.0)
        return
    end

    local vx = 0.0
    local vy = 0.0

    if is_key_pressed("a") or is_key_pressed("left") then
        vx = vx - move_speed
    end
    if is_key_pressed("d") or is_key_pressed("right") then
        vx = vx + move_speed
    end
    if is_key_pressed("w") or is_key_pressed("up") then
        vy = vy + move_speed
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
    end

    set_velocity(vx, vy)

    local mouse_down = is_mouse_pressed("left")
    local mouse_once = mouse_down and (not mouse_click_was_down)
    mouse_click_was_down = mouse_down

    if mouse_once then
        local mouse_world = get_mouse_world_position()
        if mouse_world ~= nil then
            local px = math.floor(mouse_world.x / 4)
            local py = math.floor(mouse_world.y / 4)
            create_pixel(px, py, "Sand")
            create_pixels({
                { x = px + 1, y = py, type = "Water" },
                { x = px - 1, y = py, type = "Water" },
                { x = px, y = py + 1, type = "Water" },
                { x = px, y = py - 1, type = "Water" }
            })
        end
    end

    local spawn_once
    spawn_once, spawn_key_was_down = is_pressed_once("e", spawn_key_was_down)
    if spawn_once and decoy_id == nil then
        log("Spawning decoy in front of player")
        local decoy_distance = player_half_size + spawn_padding + 12.0
        decoy_id = create_entity({
            name = "Decoy",
            transform = {
                x = entity.x + (last_facing_x * decoy_distance),
                y = entity.y + (last_facing_y * decoy_distance),
                rotation = 0.0,
                scaleX = 1.0,
                scaleY = 1.0
            },
            velocity = { vx = 10.0, vy = 0.0 },
            sprite = { texturePath = "assets/icon.bmp", width = 18.0, height = 18.0 },
            physics = {
                bodyType = "dynamic",
                fixedRotation = true,
                density = 1.0,
                friction = 0.3,
                restitution = 0.0
            }
        })
        log("Decoy created with id=" .. tostring(decoy_id))
    end

    local delete_once
    delete_once, delete_key_was_down = is_pressed_once("q", delete_key_was_down)
    if delete_once and decoy_id ~= nil then
        log("Deleting decoy id=" .. tostring(decoy_id))
        if delete_entity(decoy_id) then
            log("Decoy deleted id=" .. tostring(decoy_id))
        end
        decoy_id = nil
    end

    local toggle_once
    toggle_once, toggle_sprite_was_down = is_pressed_once("r", toggle_sprite_was_down)
    if toggle_once then
        log("Toggling player sprite")
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

    if other.name == "Mine" then
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
            log("Gate switched to unlocked visual")
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
    log("Player victory callback: " .. tostring(reason))
end

function on_lose(reason)
    log("Player lose callback: " .. tostring(reason))
end
