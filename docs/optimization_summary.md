# Performance Optimizations for The Forgotten Server

This document summarizes the performance optimizations implemented to improve server efficiency when handling large numbers of players.

## 1. String Optimization with Cached Lowercase Names

### Problem
Converting player names to lowercase for case-insensitive lookups was being performed repeatedly, especially in frequently called functions like `getPlayerByName()`.

### Solution
Added a `lowercaseName` field to the Player class that's automatically set when the name is set, avoiding redundant string transformations.

### Implementation Details
1. Added a `lowercaseName` field to the Player class 
2. Updated `Player::setName()` to automatically set the lowercase version
3. Added `Player::getLowercaseName()` accessor method
4. Modified `Game::addPlayer()` and `Game::removePlayer()` to use the pre-computed lowercase name
5. Updated `Game::getPlayerByName()` to reduce string conversions

### Performance Impact
- Eliminates redundant string conversions when accessing player data
- Reduces CPU usage for name lookups in high-traffic operations
- Particularly significant with many players online (1000+)

## 2. Grid-Based Spatial Partitioning

Added a grid system to efficiently track and find players in the world, significantly reducing the cost of finding players within a certain area.

### Key functions:
- `updatePlayerRegion`: Updates a player's position in the grid when moving
- `getSpectatorsInternal`: Uses the grid for efficient spectator lookup
- `getSpectators`: Optimized to use a 3x3 grid search for player-only lookups
- Integrated with existing movement and placement systems

## 3. Batched Network Updates

Implemented a system to batch visual effects and reduce network packets by sending multiple updates in a single message.

### Components:
- Added `Update` struct for different types of updates (magic effects, health, text)
- Added `pendingUpdates` vector to Player class
- Modified Game methods to queue updates instead of sending immediately
- Added code to send batched updates at the end of a game tick

### Memory Management:
- Implemented proper move semantics in the Update struct for efficient vector operations
- Added proper destructor to cleanup dynamic resources
- Prevented object slicing by deleting copy operations
- Used union for memory-efficient storage of different update types

### Network Protection:
- Added configurable network queue size limit to prevent memory overflow
- Implemented queue size checking in `Game::enqueueNetworkUpdate`
- Added configuration option in config.lua for adjusting the network queue size
- Network updates are dropped when the queue is full, preventing memory exhaustion

## 4. Data Structure Optimization

### Problem
Using `std::map` for player lookups resulted in O(log n) complexity, becoming a bottleneck with many players.

### Solution
Replaced `std::map` with `std::unordered_map` for O(1) average lookup time.

### Implementation Details
- Changed containers for players, monsters, NPCs and other frequently accessed collections
- Added custom hash function for pointer keys
- Maintained same interface but with faster lookup performance
- Converted additional maps to unordered_maps for better lookup performance:
  - Monster containers (monsters, unloadedMonsters, elementMap)
  - House containers (HouseMap)
  - Town containers (TownMap)
  - Player containers (openContainers, depotLockerMap, depotChests)
  - Chat containers (normalChannels, privateChannels, partyChannels, guildChannels)
  - Map containers (SpectatorCache, waypoints)
  - Action and TalkAction containers
  - Weapon containers
  - Vocation containers
  - Added necessary hash functions for Position and other custom types

## Performance Measurement

These optimizations collectively reduce:
- CPU usage from inefficient string operations and needless conversions
- Memory usage from redundant string objects
- Network overhead by batching updates
- Lookup time for finding players and creatures

The performance improvements are particularly noticeable during high-traffic operations with many players online. 