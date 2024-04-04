function Creature:getCreature() return self end

function Creature:getPlayer() return Player(self) end

function Creature:getMonster() return Monster(self) end

function Creature:getNpc() return Npc(self) end

function Creature:getTeleport() return nil end

function Creature:isContainer() return false end

function Creature:isItem() return false end

function Creature:getItem() return nil end

function Creature:isMonster() return false end

function Creature:isNpc() return false end

function Creature:isPlayer() return false end

function Creature:isTeleport() return false end

function Creature:isTile() return false end

function Creature:getClosestFreePosition(position, maxRadius, mustBeReachable)
	maxRadius = maxRadius or 1

	-- backward compatability (extended)
	if maxRadius == true then maxRadius = 2 end

	local checkPosition = Position(position)
	for radius = 0, maxRadius do
		checkPosition.x = checkPosition.x - math.min(1, radius)
		checkPosition.y = checkPosition.y + math.min(1, radius)

		local total = math.max(1, radius * 8)
		for i = 1, total do
			if radius > 0 then
				local direction = math.floor((i - 1) / (radius * 2))
				checkPosition:getNextPosition(direction)
			end

			local tile = Tile(checkPosition)
			if tile and tile:getCreatureCount() == 0 and not tile:hasProperty(CONST_PROP_IMMOVABLEBLOCKSOLID) and
				(not mustBeReachable or self:getPathTo(checkPosition)) then return checkPosition end
		end
	end
	return Position()
end

function Creature:setMonsterOutfit(monster, time)
	local monsterType = MonsterType(monster)
	if not monsterType then return false end

	local player = self:getPlayer()
	if player and not (player:hasFlag(PlayerFlag_CanIllusionAll) or monsterType:isIllusionable()) then
		return false
	end

	local condition = Condition(CONDITION_OUTFIT)
	condition:setOutfit(monsterType:getOutfit())
	condition:setTicks(time)
	self:addCondition(condition)

	return true
end

function Creature:setItemOutfit(item, time)
	local itemType = ItemType(item)
	if not itemType then return false end

	local condition = Condition(CONDITION_OUTFIT)
	condition:setOutfit({lookTypeEx = itemType:getId()})
	condition:setTicks(time)
	self:addCondition(condition)

	return true
end

function Creature:addSummon(monster)
	local summon = Monster(monster)
	if not summon then return false end

	summon:setTarget(nil)
	summon:setFollowCreature(nil)
	summon:setDropLoot(false)
	summon:setSkillLoss(false)
	summon:setMaster(self)

	if self:isPlayer() then summon:getPosition():notifySummonAppear(summon) end

	return true
end

function Creature:removeSummon(monster)
	local summon = Monster(monster)
	if not summon or summon:getMaster() ~= self then return false end

	summon:setTarget(nil)
	summon:setFollowCreature(nil)
	summon:setDropLoot(true)
	summon:setSkillLoss(true)
	summon:setMaster(nil)

	return true
end

function Creature:addDamageCondition(target, type, list, damage, period, rounds)
	if damage <= 0 or not target or target:isImmune(type) then return false end

	local condition = Condition(type)
	condition:setParameter(CONDITION_PARAM_OWNER, self:getId())
	condition:setParameter(CONDITION_PARAM_DELAYED, 1)

	if list == DAMAGELIST_EXPONENTIAL_DAMAGE then
		local exponent, value = -10, 0
		while value < damage do
			value = math.floor(10 * (-1.2 ^ exponent) + 0.5)
			condition:addDamage(1, period or 4000, -value)

			if value >= damage then
				local permille = math.random(10, 1200) / 1000
				condition:addDamage(1, period or 4000, -math.max(1, math.floor(value * permille + 0.5)))
			else
				exponent = exponent + 1
			end
		end
	elseif list == DAMAGELIST_LOGARITHMIC_DAMAGE then
		local n, value = 0, damage
		while value > 0 do
			value = math.floor(damage * (2.718281828459 ^ -0.05 * n) + 0.5)
			if value ~= 0 then
				condition:addDamage(1, period or 4000, -value)
				n = n + 1
			end
		end
	elseif list == DAMAGELIST_VARYING_PERIOD then
		for _ = 1, rounds do condition:addDamage(1, math.random(period[1], period[2]) * 1000, -damage) end
	elseif list == DAMAGELIST_CONSTANT_PERIOD then
		condition:addDamage(rounds, period * 1000, -damage)
	end

	target:addCondition(condition)
	return true
end

function Creature:canAccessPz()
	if self:isMonster() then return false end
	local player = self:getPlayer()
	return player and not player:isPzLocked()
end

function Creature:removeStorageValue(key) return self:setStorageValue(key, nil) end
