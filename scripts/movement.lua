-- ============================================
-- Pikselite Engine — Example movement script
-- ============================================
-- This script is called once per entity per frame.
-- The global table `entity` contains:
--   entity.id       (read-only)
--   entity.x, entity.y          (position)
--   entity.vx, entity.vy        (velocity)
--   entity.rotation
--   entity.scaleX, entity.scaleY
--
-- Available functions:
--   is_key_pressed(key)  →  "w","a","s","d","up","down","left","right","space","lshift","escape"
--   log(message)         →  prints to console
-- ============================================

local speed = 100.0

function update(dt)
    -- Reset velocity each frame
    entity.vx = 0
    entity.vy = 0

    -- Movement
    if is_key_pressed("w") or is_key_pressed("up") then
        entity.vy = -speed * dt
    end
    if is_key_pressed("s") or is_key_pressed("down") then
        entity.vy = speed * dt
    end
    if is_key_pressed("a") or is_key_pressed("left") then
        entity.vx = -speed * dt
    end
    if is_key_pressed("d") or is_key_pressed("right") then
        entity.vx = speed * dt
    end

    -- Sprint (shift = double speed)
    if is_key_pressed("lshift") then
        entity.vx = entity.vx * 2
        entity.vy = entity.vy * 2
    end

    -- Apply velocity to position
    entity.x = entity.x + entity.vx
    entity.y = entity.y + entity.vy
end
