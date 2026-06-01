-- ============================================
-- Basic chase example
-- Attach this script with the Script component.
-- Requires the chaser entity to have Transform + Velocity.
-- ============================================

local target_name = "Player"
local chase_speed = 120.0

function update(dt)
    local target = get_entity_by_name(target_name)
    if target == nil then
        return
    end

    basic_chase(entity.id, target.id, chase_speed)
end

function on_collision_enter(otherId)
    local other = get_entity(otherId)
    if other ~= nil and other.name == target_name then
        set_victory("Target caught")
    end
end
