-- ============================================
-- Pikselite Engine — Example movement script
-- ============================================
-- This script is called once per entity per frame.
-- The global table `entity` contains:
--   entity.id       (read-only)
--   entity.x, entity.y          (position, read-only in physics mode)
--   entity.vx, entity.vy        (velocity; vx is horizontal intent)
--   entity.rotation
--   entity.scaleX, entity.scaleY
--
-- Available functions:
--   is_key_pressed(key)  →  "w","a","s","d","up","down","left","right","space","lshift","escape"
--   log(message)         →  prints to console
--
-- Entity / component helpers:
--   get_entity(id)
--   get_entity_by_name(name)
--   get_entities()
--   create_entity(optionalTable)
--   delete_entity(id)
--   has_component(id, type)
--   get_component(id, type)
--   set_component(id, type, table)
--   add_component(id, type, table)
--   remove_component(id, type)
--
-- Gameplay helpers:
--   create_pixel(x, y, type)
--   create_pixels({ {x=0,y=0,type="Sand"}, ... })
--   basic_chase(selfId, targetId, speed)
--   set_victory(reason)
--   set_lose(reason)
--   is_victory()
--   is_lose()
--
-- Optional callbacks called by the engine when present:
--   on_collision_enter(otherId)
--   on_collision_exit(otherId)
--   on_victory(reason)
--   on_lose(reason)
-- ============================================

local speed = 100.0
local jump_speed = 300.0
local jump_was_pressed = false

function update(dt)
    -- Horizontal intent only; gravity is handled by PhysicsSystem.
    entity.vx = 0
    entity.vy = 0

    if is_key_pressed("a") or is_key_pressed("left") then
        entity.vx = -speed
    end
    if is_key_pressed("d") or is_key_pressed("right") then
        entity.vx = speed
    end

    local jump_pressed = is_key_pressed("space")
    if jump_pressed and not jump_was_pressed then
        entity.vy = jump_speed
    end
    jump_was_pressed = jump_pressed

    -- Sprint (shift = double speed)
    if is_key_pressed("lshift") then
        entity.vx = entity.vx * 2
    end
end

function on_collision_enter(otherId)
    log("collision enter with entity " .. tostring(otherId))
end

function on_collision_exit(otherId)
    log("collision exit with entity " .. tostring(otherId))
end

function on_victory(reason)
    log("victory: " .. tostring(reason))
end

function on_lose(reason)
    log("lose: " .. tostring(reason))
end
