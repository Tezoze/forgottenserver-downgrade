# Batched Network Updates

## Overview

This document describes the implementation of a batched network update system to optimize performance for servers with many players. Instead of sending individual network packets for each visual effect, health update, or text animation, these updates are batched together and sent as a single packet at the end of each game tick.

## Server Implementation

The following changes have been made to the server:

1. Added an `Update` struct in `player.h` to represent different types of updates (magic effects, health, animated text)
2. Added methods in the `Player` class to manage pending updates and send them in batches
3. Modified the game effects methods to queue updates instead of sending them immediately:
   - `Game::addMagicEffect`
   - `Game::addAnimatedText`
   - `Game::addCreatureHealth`
4. Modified the `Game::checkCreatures` method to send batched updates at the end of each game tick

## Required Client Changes

For this system to work, the client must be updated to parse the new batched update packet format:

1. Add a new protocol opcode (0xFF) for batched updates
2. Implement a handler for this opcode that:
   - Reads the number of updates in the batch
   - For each update:
     - Reads the update type (magic effect, health, animated text)
     - Reads the position
     - Reads type-specific data (effect ID, health value, or text+color)
     - Processes each update as if it had been received individually

### Client Protocol Example

```cpp
void ProtocolGame::parseBatchedUpdates(NetworkMessage& msg)
{
    uint16_t count = msg.getU16();
    for (uint16_t i = 0; i < count; i++) {
        uint8_t type = msg.getByte();
        Position pos = msg.getPosition();
        
        switch (type) {
            case 0: // MAGIC_EFFECT
                uint8_t effectId = msg.getByte();
                addMagicEffect(pos, effectId);
                break;
                
            case 1: // HEALTH
                uint32_t health = msg.getU32();
                updateCreatureHealth(pos, health);
                break;
                
            case 2: // ANIMATED_TEXT
                std::string text = msg.getString();
                uint8_t color = msg.getByte();
                addAnimatedText(pos, color, text);
                break;
        }
    }
}
```

## Performance Impact

This optimization reduces the number of network packets from potentially hundreds per tick to just one per player per tick, significantly reducing network overhead and system call overhead. This is especially important for servers supporting 1000+ players.

## Memory Management

The `Update` struct has been carefully designed with proper memory management:

1. It uses a union to efficiently store different types of update data
2. It implements proper move semantics with a move constructor and move assignment operator
3. It deletes copy operations to prevent accidental object slicing
4. It properly cleans up dynamically allocated resources in the destructor

## Network Queue Protection

To prevent memory overflow in case of network congestion or slow clients:

1. Added a configurable network queue size limit in `ConfigManager`
2. Modified `Game::enqueueNetworkUpdate` to check against this limit
3. When the queue is full, additional updates are dropped to prevent memory exhaustion
4. The queue size can be configured via the `networkQueueSize` setting in config.lua

## Future Enhancements

The system can be extended to support additional types of visual updates:
- Distance effects (projectiles)
- Creature icons
- Creature status changes
- Map updates

These would need to be added to the `Update` struct and handled appropriately in both the server and client code. 