-- Demo crystal script.

function update(dt)
    entity.rotation = entity.rotation + (1.6 * dt)
end

function on_collision_enter(other_id)
    local other = get_entity(other_id)
    if other ~= nil and other.name == "Player" then
        log("Crystal touched by Player")
    end
end

function on_victory(reason)
    log("Crystal victory callback: " .. tostring(reason))
end

function on_lose(reason)
    log("Crystal lose callback: " .. tostring(reason))
end
