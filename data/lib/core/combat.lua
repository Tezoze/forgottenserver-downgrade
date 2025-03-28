-- Utility Functions
function Combat:getPositions(creature, variant)
    local positions = {}
    local function callback(creature, position) positions[#positions + 1] = position end
    self:setCallback(CallBackParam.TARGETTILE, callback)
    self:execute(creature, variant)
    return positions
end

function Combat:getTargets(creature, variant)
    local targets = {}
    local function callback(creature, target) targets[#targets + 1] = target end
    self:setCallback(CallBackParam.TARGETCREATURE, callback)
    self:execute(creature, variant)
    return targets
end

-- Damage Formulas (Tibia 8.6)
function onGetMeleeDamage(player, skill, attackValue, attackFactor)
    local level = getPlayerLevel(player.uid)
    local min = level / 5
    local max = (0.085 * skill * attackValue) + (attackValue / 3)
    if attackFactor == 1.0 then -- Full Attack
        max = max
    elseif attackFactor == 5/6 then -- Balanced
        max = max * (5/6)
    else -- Defensive
        max = max * 0.5
    end
    local target = getCreatureTarget(player.uid)
    if target and isPlayer(target) then
        min = min * 0.6 -- 8.6 PvP reduction for physical
        max = max * 0.6
    end
    return min, max
end

function onGetDistanceDamage(player, skill, attackValue, attackFactor)
    local level = getPlayerLevel(player.uid)
    local min = level / 5
    local max = (0.085 * skill * attackValue) + (attackValue / 3)
    local target = getCreatureTarget(player.uid)
    if target and isPlayer(target) then
        min = min * 0.6 -- 8.6 PvP reduction for physical
        max = max * 0.6
    end
    return min, max
end

function onGetMagicDamage(player, level, maglevel)
    local min = (level / 5) + (maglevel * 1.5) -- Default for "exori vis"
    local max = (level / 5) + (maglevel * 3)
    return min, max
end

function onGetExoriVisDamage(player, level, maglevel)
    local min = (level / 5) + (maglevel * 1.5)
    local max = (level / 5) + (maglevel * 3)
    return min, max
end

function onGetExevoGranMasFlamDamage(player, level, maglevel)
    local min = (level / 5) + (maglevel * 2)
    local max = (level / 5) + (maglevel * 4)
    return min, max
end