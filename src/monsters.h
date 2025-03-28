// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once
#ifndef FS_MONSTERS_H_123456789
#define FS_MONSTERS_H_123456789

#include "creature.h"
#include <map>
#include <string>
#include <unordered_map>

class MonsterType;
class MonsterSpell;
struct LootBlock;
struct spellBlock_t;

class Monsters
{
public:
	Monsters() = default;
	~Monsters() = default;

	// non-copyable
	Monsters(const Monsters&) = delete;
	Monsters& operator=(const Monsters&) = delete;

	bool loadFromXml(bool reloading = false);
	bool reload();
	
	MonsterType* getMonsterType(const std::string& name, bool loadFromFile = true);
	MonsterType* getMonsterType(uint32_t raceId);
	
	// Loot helper functions
	bool loadLootItem(const pugi::xml_node& node, LootBlock& lootBlock);
	void loadLootContainer(const pugi::xml_node& node, LootBlock& lBlock);
	static void loadLoot(MonsterType* monsterType, LootBlock lootBlock);
	
	// Monster spell-related functions
	bool deserializeSpell(const pugi::xml_node& node, spellBlock_t& sb, const std::string& description);
	bool deserializeSpell(MonsterSpell* spell, spellBlock_t& sb, const std::string& description);
	
	// Load a specific monster from file
	MonsterType* loadMonster(const std::string& file, const std::string& monsterName, bool reloading = false);
	
	// Create conditions
	ConditionDamage* getDamageCondition(ConditionType_t conditionType, int32_t maxDamage, int32_t minDamage,
	                                   int32_t startDamage, uint32_t tickInterval);
	
	// Bestiary registration
	bool registerBestiaryMonster(const MonsterType* mType);

private:
	std::map<std::string, std::string> unloadedMonsters;
	std::map<uint32_t, MonsterType*> monsters;
	
	friend class MonsterSpell;
	friend class MonsterType;
};

extern Monsters g_monsters;

#endif // FS_MONSTERS_H_123456789
