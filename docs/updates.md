# Performance Optimization Changes

This document provides a detailed list of all changes made to optimize The Forgotten Server for handling large numbers of players.

## Grid-Based Spatial Partitioning

### Map.h
- Added `playerGrid` unordered_map to store players by region
- Added `GRID_SIZE` constant (32 units)
- Added `pair_hash` struct for hashing grid coordinates
- Added `updatePlayerRegion` method declaration
- Added `clearPlayerGrid` method
- Added `getGridStats` for debugging
- Added map layer constants for better organization:
  - `MAP_LAYER_VIEW_LIMIT`
  - `MAP_LAYER_LOWER_LIMIT`
  - `MAP_LAYER_UPPER_LIMIT`

### Map.cpp
- Implemented `updatePlayerRegion` to move players between grid cells
- Modified `getSpectatorsInternal` to use grid for player-only queries
- Added debug logging for grid operations
- Optimized `getSpectators` to use a 3x3 grid search for player-only lookups
- Simplified range calculations for more efficient player lookups
- Improved `getSpectators` to get spectators only once for the entire operation
- Fixed issue with player filtering in grid-based spectator lookups

### Game.cpp
- Modified `placeCreature` to add players to the grid
- Modified `removeCreature` to remove players from the grid
- Modified `start` to clear playerGrid on startup
- Modified `shutdown` to clear playerGrid on shutdown

## String Optimization

### Player.h
- Added `lowercaseName` string field to Player class
- Modified header includes to add boost/algorithm/string.hpp
- Added `getLowercaseName()` accessor method
- Modified `setName()` to set `lowercaseName` automatically

### Player.cpp
- Updated Player constructor with comment about lowercaseName initialization

### Game.cpp
- Modified `addPlayer` to use player->getLowercaseName()
- Modified `removePlayer` to use player->getLowercaseName()
- Updated `getPlayerByName` to reduce string conversions
- Modified other player lookup functions to leverage cached lowercase names

## Batched Network Updates

### Player.h
- Added `Update` struct with union for different update types
- Added constructor and destructor for proper memory management
- Added `pendingUpdates` vector to Player class
- Added `sendPendingUpdates()` method
- Added `clearPendingUpdates()` method

### Player.cpp
- Implemented `sendPendingUpdates()` to send batched updates
- Added code to handle different update types (magic effects, health, text)
- Optimized `sendPendingUpdates` to use move semantics for better performance

### Game.cpp
- Modified `addMagicEffect` to queue updates instead of sending immediately
- Modified `addAnimatedText` to queue text effects
- Modified `addCreatureHealth` to queue health updates
- Modified `checkCreatures` to call sendPendingUpdates at the end of a tick
- Improved effect and text update methods to use move semantics with pending updates
- Simplified network message handling with efficient memory management

## Data Structure Optimization

### Game.h
- Added hash function for Item* pointers
- Changed containers from std::map to std::unordered_map:
  - players
  - monsters
  - npcs
  - mappedPlayerNames
  - mappedPlayerGuids
  - guilds
  - uniqueItems
  - tradeItems
  - bedSleepersMap

### Type Changes
- Container key types to support unordered maps:
  - string to std::string for player names
  - Position to std::pair<int,int> for grid coordinates

### Extended Map Replacements
- Replaced additional `std::map` with `std::unordered_map` for O(1) lookups:
  - `MonsterType::MonsterInfo::elementMap`
  - `Monsters::monsters` 
  - `Monsters::unloadedMonsters`
  - `HouseMap` typedef
  - `TownMap` typedef
  - `Player::openContainers`
  - `Player::depotLockerMap`
  - `Player::depotChests`
  - `MuteCountMap` typedef
  - `ActionUseMap` typedef
  - `Weapons::weapons`
  - `VocationMap` typedef
  - `UsersMap` and `InvitedMap` (Chat system)
  - `Chat` class container maps
  - `Map::waypoints`
  - `SpectatorCache`

### Position Hash Function
- Added hash function for Position class to support unordered_map usage
- Added necessary header includes to support unordered_map in various files

## Documentation
- Created optimization_summary.md with overview of changes
- Added debugging and statistics functions
- Created batched_updates.md document explaining network updates batching
- Updated updates.md with detailed changes

## Code Cleanup
- Fixed explicit capture in lambda expressions
- Standardized naming conventions
- Added comments for clarity
- Ensured proper memory management for the Update struct

## Memory Management Optimization

### Game.h
- Added object pooling system for temporary items
- Added `itemPool` vector to store reusable Item objects
- Added `itemPoolMutex` for thread safety
- Added method declarations for pool management:
  - `getPooledItem(uint16_t id)` 
  - `returnPooledItem(Item* item)`
  - `createItem(uint16_t itemId, uint16_t count)`

### Game.cpp
- Implemented `getPooledItem` to retrieve items from the pool or create new ones
- Implemented `returnPooledItem` to return items to the pool for reuse
- Implemented `createItem` as a convenient wrapper for item creation
- Updated item creation in combat code to use pooled items
- Modified `combatGetTypeInfo` to use pooled items for splash effects

### ConfigManager
- Added `ITEM_POOL_SIZE` setting to make item pool size configurable
- Added default value of 1000 for pool size
- Updated config loading to read itemPoolSize from config.lua
- Added `NETWORK_QUEUE_SIZE` integer setting with default value of 100
- Updated config.lua with networkQueueSize setting

## Staggered Processing

### Game.h
- Added `checkCreaturesChunk` method for distributed entity processing
- Modified existing creature processing methods to support staggered execution
- Added mutexes for thread-safe creature access:
  - `playerMutex`
  - `monsterMutex`
  - `npcMutex`

### Game.cpp
- Implemented `checkCreaturesChunk` to process a portion of creatures each tick
- Modified `checkCreatures` to divide work into chunks across multiple ticks
- Added distribution logic to evenly spread processing load
- Improved chunk processing with snapshots to prevent iterator invalidation
- Changed chunk scheduling to use evenly spaced intervals
- Modified `start` method to space chunks evenly based on think interval
- Moved sendPendingUpdates inside player processing loop for better batching

## Thread Safety & Multithreading

### Game.h
- Added `networkMutex` for thread-safe network operations
- Added `networkCondition` condition variable to avoid busy-waiting
- Added network thread related methods:
  - `enqueueNetworkUpdate`
  - `networkThread`
  - `startNetworkThread`
  - `stopNetworkThread`

### Game.cpp
- Implemented dedicated network thread for asynchronous packet sending
- Created thread-safe queue for network messages
- Added synchronization for item pool access
- Added proper thread start/stop handling in game startup and shutdown
- Used locking to protect shared resources during concurrent access
- Optimized networkThread to use condition_variable instead of sleep-based polling
- Enhanced thread shutdown to properly signal waiting threads
- Added network queue size limit to prevent memory overflow
- Fixed enqueueNetworkUpdate to handle queue overflow more gracefully
- Simplified queue handling to avoid using unsupported queue operations

## Memory Management Enhancements

### Player.h
- Improved Update struct with proper move semantics
- Added proper cleanup for dynamically allocated resources
- Prevented object slicing by deleting copy operations
- Implemented resource acquisition is initialization (RAII) pattern

### Player.cpp
- Optimized sendPendingUpdates for move semantics
- Improved resource management in network update handling
- Updated method to use C++17 features for cleaner code
