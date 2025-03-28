// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "networkmessage.h"

#include "container.h"
#include "creature.h"

std::string_view NetworkMessage::getString(uint16_t stringLen /* = 0*/)
{
	if (stringLen == 0) {
		// Get string length from the message
		if (!canRead(sizeof(uint16_t))) {
			return "";
		}
		stringLen = get<uint16_t>();
	}

	if (stringLen > MAX_STRING_LENGTH) {
		info.overrun = true;
		return "";
	}

	if (!canRead(stringLen)) {
		return "";
	}

	auto it = buffer.data() + info.position;
	info.position += stringLen;
	return {reinterpret_cast<char*>(it), stringLen};
}

Position NetworkMessage::getPosition()
{
	Position pos;
	// Check if we can read the full position (4 bytes for x,y and 1 byte for z)
	if (!canRead(sizeof(uint16_t) * 2 + sizeof(uint8_t))) {
		return pos; // Return default position (0,0,0) if can't read
	}
	
	pos.x = get<uint16_t>();
	pos.y = get<uint16_t>();
	pos.z = getByte();
	return pos;
}

void NetworkMessage::addString(std::string_view value)
{
	size_t stringLen = value.length();
	if (stringLen > MAX_STRING_LENGTH) {
		return;
	}

	if (!canAdd(stringLen + 2)) {
		return;
	}

	add<uint16_t>(static_cast<uint16_t>(stringLen));
	if (stringLen > 0) {
		std::memcpy(buffer.data() + info.position, value.data(), stringLen);
		info.position += stringLen;
		info.length += stringLen;
	}
}

void NetworkMessage::addDouble(double value, uint8_t precision /* = 2*/)
{
	if (precision > 10) {
		precision = 10;
	}

	addByte(precision);
	
	// Calculate with proper bounds checking to avoid overflow
	double multiplier = std::pow(10.0, static_cast<double>(precision));
	double transformed = value * multiplier;
	
	// Clamp to int32_t range to avoid overflow
	constexpr double max_val = static_cast<double>(std::numeric_limits<int32_t>::max());
	constexpr double min_val = static_cast<double>(std::numeric_limits<int32_t>::min());
	
	if (transformed > max_val) {
		transformed = max_val;
	} else if (transformed < min_val) {
		transformed = min_val;
	}
	
	add<uint32_t>(static_cast<uint32_t>(transformed + std::numeric_limits<int32_t>::max()));
}

void NetworkMessage::addBytes(const char* bytes, size_t size)
{
	if (!bytes || size == 0) {
		return;
	}

	if (!canAdd(size) || size > MAX_STRING_LENGTH) {
		return;
	}

	std::memcpy(buffer.data() + info.position, bytes, size);
	info.position += size;
	info.length += size;
}

void NetworkMessage::addPaddingBytes(size_t n)
{
	if (n == 0) {
		return;
	}

	if (!canAdd(n)) {
		return;
	}

	std::fill_n(buffer.data() + info.position, n, 0x33);
	info.position += n;
	info.length += n;
}

void NetworkMessage::addPosition(const Position& pos)
{
	// Check if we can add all position components at once
	if (!canAdd(sizeof(uint16_t) * 2 + sizeof(uint8_t))) {
		return;
	}
	
	add<uint16_t>(pos.x);
	add<uint16_t>(pos.y);
	addByte(pos.z);
}

void NetworkMessage::addItemId(uint16_t itemId, const bool isOTCv8)
{
	// Validate itemId to prevent out-of-bounds access
	if (itemId >= Item::items.size()) {
		add<uint16_t>(0); // Add a placeholder for invalid items
		return;
	}

	const ItemType& it = Item::items[itemId];
	uint16_t clientId = it.clientId;
	if (!isOTCv8 && itemId > 12660) {
		clientId = it.stackable ? 3031 : 105;
	}

	add<uint16_t>(clientId);
}

void NetworkMessage::addItem(uint16_t id, uint8_t count, const bool isOTCv8)
{
	// Validate item ID
	if (id >= Item::items.size()) {
		add<uint16_t>(0); // Add a placeholder for invalid items
		return;
	}

	addItemId(id, isOTCv8);

	const ItemType& it = Item::items[id];
	if (it.stackable) {
		addByte(count);
	} else if (it.isSplash() || it.isFluidContainer()) {
		// Ensure we're accessing valid indices in fluidMap
		addByte(fluidMap[count & 7]);
	}
}

void NetworkMessage::addItem(const Item* item, const bool isOTCv8)
{
	if (!item) {
		// Add a placeholder for null items to maintain protocol integrity
		add<uint16_t>(0);
		return;
	}

	uint16_t id = item->getID();
	// Validate item ID
	if (id >= Item::items.size()) {
		add<uint16_t>(0); // Add a placeholder for invalid items
		return;
	}

	addItemId(id, isOTCv8);

	const ItemType& it = Item::items[id];
	if (it.stackable) {
		addByte(static_cast<uint8_t>(std::min<uint16_t>(0xFF, item->getItemCount())));
	} else if (it.isSplash() || it.isFluidContainer()) {
		// Ensure we're accessing valid indices in fluidMap
		addByte(fluidMap[item->getFluidType() & 7]);
	}
}

void NetworkMessage::addU16(uint16_t value)
{
	if ((info.position + 2) >= NETWORKMESSAGE_MAXSIZE) {
		info.overrun = true;
		return;
	}

	buffer[info.position++] = static_cast<uint8_t>(value);
	buffer[info.position++] = static_cast<uint8_t>(value >> 8);
}

void NetworkMessage::addU32(uint32_t value)
{
	if ((info.position + 4) >= NETWORKMESSAGE_MAXSIZE) {
		info.overrun = true;
		return;
	}

	buffer[info.position++] = static_cast<uint8_t>(value);
	buffer[info.position++] = static_cast<uint8_t>(value >> 8);
	buffer[info.position++] = static_cast<uint8_t>(value >> 16);
	buffer[info.position++] = static_cast<uint8_t>(value >> 24);
}
