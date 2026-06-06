-- Demo hunter AI script.

local target_name = "Player"
local chase_speed = 110.0

function update(dt)
    if is_victory() or is_lose() then
        if has_component(entity.id, "velocity") then
            local v = get_component(entity.id, "velocity")
            v.vx = 0.0
            v.vy = 0.0
            set_component(entity.id, "velocity", v)
        end
        return
    end

    local target = get_entity_by_name(target_name)
    if target ~= nil then
        basic_chase(entity.id, target.id, chase_speed)
    end
end

function on_collision_enter(other_id)
    local other = get_entity(other_id)
    if other ~= nil and other.name == target_name then
        set_lose("Hunter caught player")
    end
end

function on_collision_exit(other_id)
    local other = get_entity(other_id)
    if other ~= nil then
        log("Hunter collision exit: " .. tostring(other.name))
    end
end

function on_victory(reason)
    log("Hunter got victory event: " .. tostring(reason))
end

function on_lose(reason)
    log("Hunter got lose event: " .. tostring(reason))
end
