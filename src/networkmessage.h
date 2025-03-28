// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_NETWORKMESSAGE_H
#define FS_NETWORKMESSAGE_H

#include "const.h"

class Item;
class Creature;
class Player;
struct Position;
class RSA;

class NetworkMessage
{
public:
	using MsgSize_t = uint16_t;
	// Headers:
	// 2 bytes for unencrypted message size
	// 4 bytes for checksum
	// 2 bytes for encrypted message size
	static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 8;
	enum
	{
		HEADER_LENGTH = 2
	};
	enum
	{
		CHECKSUM_LENGTH = 4
	};
	enum
	{
		XTEA_MULTIPLE = 8
	};
	enum
	{
		MAX_BODY_LENGTH = NETWORKMESSAGE_MAXSIZE - HEADER_LENGTH - CHECKSUM_LENGTH - XTEA_MULTIPLE
	};
	enum
	{
		MAX_PROTOCOL_BODY_LENGTH = MAX_BODY_LENGTH - 10
	};
	
	// Maximum allowed string length to prevent buffer overflows
	static constexpr size_t MAX_STRING_LENGTH = 8192;

protected:
	struct NetworkMessageInfo
	{
		MsgSize_t length = 0;
		MsgSize_t position = INITIAL_BUFFER_POSITION;
		bool overrun = false;
	};

	NetworkMessageInfo info = {};
	std::array<uint8_t, NETWORKMESSAGE_MAXSIZE> buffer = {};

public:
	NetworkMessage() = default;

	void reset() { info = {}; }

	// simply read functions for incoming message
	uint8_t getByte()
	{
		if (!canRead(1)) {
			return 0;
		}

		return buffer[info.position++];
	}

	uint8_t getPreviousByte() 
	{ 
		if (info.position <= INITIAL_BUFFER_POSITION) {
			info.overrun = true;
			return 0;
		}
		return buffer[--info.position]; 
	}

	template <typename T>
	std::enable_if_t<std::is_trivially_copyable_v<T>, T> get() noexcept
	{
		static_assert(std::is_trivially_constructible_v<T>, "Destination type must be trivially constructible");

		if (!canRead(sizeof(T))) {
			return 0;
		}

		T value;
		std::memcpy(&value, buffer.data() + info.position, sizeof(T));
		info.position += sizeof(T);
		return value;
	}

	std::string_view getString(uint16_t stringLen = 0);
	Position getPosition();

	// skips count unknown/unused bytes in an incoming message
	void skipBytes(int16_t count) 
	{ 
		if (count < 0 && static_cast<size_t>(std::abs(count)) > info.position) {
			info.position = INITIAL_BUFFER_POSITION;
			info.overrun = true;
			return;
		}
		
		if (count > 0 && !canRead(count)) {
			return;
		}
		
		info.position += count; 
	}

	// simply write functions for outgoing message
	void addByte(uint8_t value)
	{
		if (!canAdd(1)) {
			return;
		}

		buffer[info.position++] = value;
		info.length++;
	}

	template <typename T>
	void add(T value)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Value type must be trivially copyable");
		
		if (!canAdd(sizeof(T))) {
			return;
		}

		std::memcpy(buffer.data() + info.position, &value, sizeof(T));
		info.position += sizeof(T);
		info.length += sizeof(T);
	}

	void addBytes(const char* bytes, size_t size);
	void addPaddingBytes(size_t n);

	void addString(std::string_view value);

	void addDouble(double value, uint8_t precision = 2);

	// write functions for complex types
	void addPosition(const Position& pos);
	void addItemId(uint16_t itemId, const bool isOTCv8);
	void addItem(uint16_t id, uint8_t count, const bool isOTCv8);
	void addItem(const Item* item, const bool isOTCv8);

	void addU16(uint16_t value);
	void addU32(uint32_t value);

	MsgSize_t getLength() const { return info.length; }

	void setLength(MsgSize_t newLength) { info.length = newLength; }

	MsgSize_t getBufferPosition() const { return info.position; }

	bool setBufferPosition(MsgSize_t pos)
	{
		if (pos < NETWORKMESSAGE_MAXSIZE - INITIAL_BUFFER_POSITION) {
			info.position = pos + INITIAL_BUFFER_POSITION;
			return true;
		}
		return false;
	}

	uint16_t getLengthHeader() const { return static_cast<uint16_t>(buffer[0] | buffer[1] << 8); }

	bool isOverrun() const { return info.overrun; }

	uint8_t* getBuffer() { return buffer.data(); }
	const uint8_t* getBuffer() const { return buffer.data(); }

	uint8_t* getBodyBuffer()
	{
		info.position = 2;
		return &buffer[HEADER_LENGTH];
	}

private:
	bool canAdd(size_t size) const 
	{ 
		return (size + info.position) < MAX_BODY_LENGTH && size <= MAX_BODY_LENGTH; 
	}

	bool canRead(int32_t size)
	{
		if ((info.position + size) > (info.length + 8) || size >= (NETWORKMESSAGE_MAXSIZE - info.position)) {
			info.overrun = true;
			return false;
		}
		return true;
	}
};

#endif // FS_NETWORKMESSAGE_H
