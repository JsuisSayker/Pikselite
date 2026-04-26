-- Demo manager script to exercise utility APIs.

local initialized = false
local mine_id = nil
local elapsed = 0.0
local last_entity_count_log = 0.0

function update(dt)
    elapsed = elapsed + dt

    if not initialized then
        initialized = true

        local list = get_entities()
        log("Initial entity count=" .. tostring(#list))

        create_pixels({
            { x = 50, y = 20, type = "Sand" },
            { x = 51, y = 20, type = "Sand" },
            { x = 52, y = 20, type = "Sand" },
            { x = 51, y = 21, type = "Water" }
        })

        mine_id = create_entity({
            name = "TimedMine",
            transform = { x = 420.0, y = 250.0, rotation = 0.0, scaleX = 1.0, scaleY = 1.0 },
            sprite = { texturePath = "assets/icon.bmp", width = 14.0, height = 14.0 },
            physics = {
                bodyType = "static",
                fixedRotation = true,
                density = 1.0,
                friction = 0.0,
                restitution = 0.0
            }
        })

        log("TimedMine spawned id=" .. tostring(mine_id))
    end

    if elapsed - last_entity_count_log > 4.0 then
        last_entity_count_log = elapsed
        local list = get_entities()
        log("Runtime entity count=" .. tostring(#list))
    end

    if mine_id ~= nil and elapsed > 12.0 then
        if delete_entity(mine_id) then
            log("TimedMine deleted after timeout")
        end
        mine_id = nil
    end
end

function on_victory(reason)
    log("GameManager victory callback: " .. tostring(reason))
end

function on_lose(reason)
    log("GameManager lose callback: " .. tostring(reason))
end
