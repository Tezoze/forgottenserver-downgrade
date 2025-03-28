// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_ACTIONS_H
#define FS_ACTIONS_H

#include "baseevents.h"
#include "enums.h"
#include "luascript.h"
#include <unordered_map>

class Action;
using Action_ptr = std::unique_ptr<Action>;
using ActionFunction = std::function<bool(Player* player, Item* item, const Position& fromPosition, Thing* target,
                                          const Position& toPosition, bool isHotkey)>;

class Action : public Event
{
public:
	explicit Action(LuaScriptInterface* interface);

	bool configureEvent(const pugi::xml_node& node) override;
	bool loadFunction(const pugi::xml_attribute& attr, bool isScripted) override;

	// scripting
	virtual bool executeUse(Player* player, Item* item, const Position& fromPosition, Thing* target,
	                        const Position& toPosition, bool isHotkey);

	bool getAllowFarUse() const { return allowFarUse; }
	void setAllowFarUse(bool v) { allowFarUse = v; }

	bool getCheckLineOfSight() const { return checkLineOfSight; }
	void setCheckLineOfSight(bool v) { checkLineOfSight = v; }

	bool getCheckFloor() const { return checkFloor; }
	void setCheckFloor(bool v) { checkFloor = v; }

	auto stealItemIdRange()
	{
		std::vector<uint16_t> ret{};
		std::swap(ids, ret);
		return ret;
	}
	void addItemId(uint16_t id) { ids.emplace_back(id); }

	auto stealUniqueIdRange()
	{
		std::vector<uint16_t> ret{};
		std::swap(uids, ret);
		return ret;
	}
	void addUniqueId(uint16_t id) { uids.emplace_back(id); }

	auto stealActionIdRange()
	{
		std::vector<uint16_t> ret{};
		std::swap(aids, ret);
		return ret;
	}
	void addActionId(uint16_t id) { aids.emplace_back(id); }

	virtual ReturnValue canExecuteAction(const Player* player, const Position& toPos);
	virtual bool hasOwnErrorHandler() { return false; }
	virtual Thing* getTarget(Player* player, Creature* targetCreature, const Position& toPosition,
	                         uint8_t toStackPos) const;

	ActionFunction function;

private:
	std::string_view getScriptEventName() const override;

	bool allowFarUse = false;
	bool checkFloor = true;
	bool checkLineOfSight = true;
	std::vector<uint16_t> ids;
	std::vector<uint16_t> uids;
	std::vector<uint16_t> aids;
};

class Actions final : public BaseEvents
{
public:
	Actions();
	~Actions();

	// non-copyable
	Actions(const Actions&) = delete;
	Actions& operator=(const Actions&) = delete;

	bool useItem(Player* player, const Position& pos, uint8_t index, Item* item, bool isHotkey);
	bool useItemEx(Player* player, const Position& fromPos, const Position& toPos, uint8_t toStackPos, Item* item,
	               bool isHotkey, Creature* creature = nullptr);

	ReturnValue canUse(const Player* player, const Position& pos);
	ReturnValue canUse(const Player* player, const Position& pos, const Item* item);
	ReturnValue canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight, bool checkFloor);

	bool registerLuaEvent(Action* event);
	void clear(bool fromLua) override final;

private:
	ReturnValue internalUseItem(Player* player, const Position& pos, uint8_t index, Item* item, bool isHotkey);

	LuaScriptInterface& getScriptInterface() override;
	std::string_view getScriptBaseName() const override;
	Event_ptr getEvent(std::string_view nodeName) override;
	bool registerEvent(Event_ptr event, const pugi::xml_node& node) override;

	using ActionUseMap = std::unordered_map<uint16_t, Action>;
	ActionUseMap useItemMap;
	ActionUseMap uniqueItemMap;
	ActionUseMap actionItemMap;

	Action* getAction(const Item* item);
	void clearMap(ActionUseMap& map, bool fromLua);

	LuaScriptInterface scriptInterface;
};

#endif
