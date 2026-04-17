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
