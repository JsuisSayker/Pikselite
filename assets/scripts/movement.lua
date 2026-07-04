local move_speed = 130.0
local jump_high = 500.0

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

function update(dt)
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

    set_velocity(vx, vy)
end
