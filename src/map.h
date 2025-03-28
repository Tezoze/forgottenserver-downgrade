// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_MAP_H
#define FS_MAP_H

#include "otpch.h"

#include "house.h"
#include "position.h"
#include "spawn.h"
#include "spectators.h"
#include "town.h"

class Creature;
class Player;

inline constexpr int32_t MAP_MAX_LAYERS = 16;

// Map layer limits
inline constexpr int32_t MAP_LAYER_VIEW_LIMIT = 7;
inline constexpr int32_t MAP_LAYER_LOWER_LIMIT = 0;
inline constexpr int32_t MAP_LAYER_UPPER_LIMIT = 15;

// Grid size for spatial partitioning (32x32 tiles per grid cell)
inline constexpr int32_t GRID_SIZE = 32;

// Add grid-based spatial partitioning
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator()(const std::pair<T1, T2>& p) const {
		return std::hash<T1>{}(p.first) ^ std::hash<T2>{}(p.second);
	}
};

struct FindPathParams;
struct AStarNode
{
	AStarNode* parent;
	int_fast32_t f;
	uint16_t x, y;
};

inline constexpr int32_t MAX_NODES = 512;

inline constexpr int32_t MAP_NORMALWALKCOST = 10;
inline constexpr int32_t MAP_DIAGONALWALKCOST = 25;

class AStarNodes
{
public:
	AStarNodes(uint32_t x, uint32_t y);

	AStarNode* createOpenNode(AStarNode* parent, uint32_t x, uint32_t y, int_fast32_t f);
	AStarNode* getBestNode();
	void closeNode(AStarNode* node);
	void openNode(AStarNode* node);
	int_fast32_t getClosedNodes() const;
	AStarNode* getNodeByPosition(uint32_t x, uint32_t y);

	static int_fast32_t getMapWalkCost(AStarNode* node, const Position& neighborPos);
	static int_fast32_t getTileWalkCost(const Creature& creature, const Tile* tile);

private:
	AStarNode nodes[MAX_NODES];
	bool openNodes[MAX_NODES];
	std::unordered_map<uint32_t, AStarNode*> nodeTable;
	size_t curNode;
	int_fast32_t closedNodes;
};

// Position hash function for unordered_map
namespace std {
	template<>
	struct hash<Position> {
		std::size_t operator()(const Position& pos) const {
			// Combine the hash of the coordinates
			std::size_t h1 = std::hash<int32_t>{}(pos.x);
			std::size_t h2 = std::hash<int32_t>{}(pos.y);
			std::size_t h3 = std::hash<int32_t>{}(pos.z);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};
}

using SpectatorCache = std::unordered_map<Position, SpectatorVec>;

inline constexpr int32_t FLOOR_BITS = 3;
inline constexpr int32_t FLOOR_SIZE = (1 << FLOOR_BITS);
inline constexpr int32_t FLOOR_MASK = (FLOOR_SIZE - 1);

struct Floor
{
	constexpr Floor() = default;
	~Floor();

	// non-copyable
	Floor(const Floor&) = delete;
	Floor& operator=(const Floor&) = delete;

	Tile* tiles[FLOOR_SIZE][FLOOR_SIZE] = {};
};

class FrozenPathingConditionCall;
class QTreeLeafNode;

class QTreeNode
{
public:
	constexpr QTreeNode() = default;
	virtual ~QTreeNode();

	// non-copyable
	QTreeNode(const QTreeNode&) = delete;
	QTreeNode& operator=(const QTreeNode&) = delete;

	bool isLeaf() const { return leaf; }

	QTreeLeafNode* getLeaf(uint32_t x, uint32_t y);

	template <typename Leaf, typename Node>
	static Leaf getLeafStatic(Node node, uint32_t x, uint32_t y)
	{
		do {
			node = node->child[((x & 0x8000) >> 15) | ((y & 0x8000) >> 14)];
			if (!node) {
				return nullptr;
			}

			x <<= 1;
			y <<= 1;
		} while (!node->leaf);
		return static_cast<Leaf>(node);
	}

	QTreeLeafNode* createLeaf(uint32_t x, uint32_t y, uint32_t level);

protected:
	bool leaf = false;

private:
	QTreeNode* child[4] = {};

	friend class Map;
};

class QTreeLeafNode final : public QTreeNode
{
public:
	QTreeLeafNode()
	{
		leaf = true;
		newLeaf = true;
	}
	~QTreeLeafNode();

	// non-copyable
	QTreeLeafNode(const QTreeLeafNode&) = delete;
	QTreeLeafNode& operator=(const QTreeLeafNode&) = delete;

	Floor* createFloor(uint32_t z);
	Floor* getFloor(uint8_t z) const { return array[z]; }

	void addCreature(Creature* c);
	void removeCreature(Creature* c);

private:
	static bool newLeaf;
	QTreeLeafNode* leafS = nullptr;
	QTreeLeafNode* leafE = nullptr;
	Floor* array[MAP_MAX_LAYERS] = {};
	CreatureVector creature_list;
	CreatureVector player_list;

	friend class Map;
	friend class QTreeNode;
};

/**
 * Map class.
 * Holds all the actual map-data
 */

class Map
{
public:
	static constexpr int32_t maxViewportX = 11; // min value: maxClientViewportX + 1
	static constexpr int32_t maxViewportY = 11; // min value: maxClientViewportY + 1
	static constexpr int32_t maxClientViewportX = 8;
	static constexpr int32_t maxClientViewportY = 6;

	static constexpr int32_t GRID_SIZE = 32; // Adjust based on map size and player density
	void updatePlayerRegion(Player* player, const Position& oldPos, const Position& newPos);

	uint32_t clean() const;

	/**
	 * Load a map.
	 * \returns true if the map was loaded successfully
	 */
	bool loadMap(const std::string& identifier, bool loadHouses);

	/**
	 * Save a map.
	 * \returns true if the map was saved successfully
	 */
	static bool save();

	/**
	 * Get a single tile.
	 * \returns A pointer to that tile.
	 */
	Tile* getTile(uint16_t x, uint16_t y, uint8_t z) const;
	Tile* getTile(const Position& pos) const { return getTile(pos.x, pos.y, pos.z); }

	/**
	 * Set a single tile.
	 */
	void setTile(uint16_t x, uint16_t y, uint8_t z, Tile* newTile);
	void setTile(const Position& pos, Tile* newTile) { setTile(pos.x, pos.y, pos.z, newTile); }

	/**
	 * Removes a single tile.
	 */
	void removeTile(uint16_t x, uint16_t y, uint8_t z);
	void removeTile(const Position& pos) { removeTile(pos.x, pos.y, pos.z); }

	/**
	 * Place a creature on the map
	 * \param centerPos The position to place the creature
	 * \param creature Creature to place on the map
	 * \param extendedPos If true, the creature will in first-hand be placed 2
	 * tiles away \param forceLogin If true, placing the creature will not fail
	 * because of obstacles (creatures/chests)
	 */
	bool placeCreature(const Position& centerPos, Creature* creature, bool extendedPos = false,
	                   bool forceLogin = false);

	void moveCreature(Creature& creature, Tile& newTile, bool forceTeleport = false);

	void getSpectators(SpectatorVec& spectators, const Position& centerPos, bool multifloor = false,
	                   bool onlyPlayers = false, int32_t minRangeX = 0, int32_t maxRangeX = 0, int32_t minRangeY = 0,
	                   int32_t maxRangeY = 0);

	void clearSpectatorCache();
	void clearPlayersSpectatorCache();
	void clearPlayerGrid() { playerGrid.clear(); }
	SpectatorCache& getSpectatorCache();

	// Debug function to get grid statistics
	std::string getGridStats() const {
		size_t totalPlayers = 0;
		size_t emptyRegions = 0;
		size_t maxPlayersInRegion = 0;
		
		for (const auto& [region, players] : playerGrid) {
			totalPlayers += players.size();
			if (players.empty()) {
				emptyRegions++;
			}
			maxPlayersInRegion = std::max(maxPlayersInRegion, players.size());
		}
		
		std::stringstream ss;
		ss << "Grid stats: " << playerGrid.size() << " regions, "
		   << totalPlayers << " players, "
		   << emptyRegions << " empty regions, "
		   << "max " << maxPlayersInRegion << " players in a region";
		return ss.str();
	}

	/**
	 * Checks if you can throw an object to that position
	 *	\param fromPos from Source point
	 *	\param toPos Destination point
	 *	\param rangex maximum allowed range horizontally
	 *	\param rangey maximum allowed range vertically
	 *	\param checkLineOfSight checks if there is any blocking objects in the
	 *way \param sameFloor checks if the destination is on same floor \returns
	 *The result if you can throw there or not
	 */
	bool canThrowObjectTo(const Position& fromPos, const Position& toPos, bool checkLineOfSight = true,
	                      bool sameFloor = false, int32_t rangex = Map::maxClientViewportX,
	                      int32_t rangey = Map::maxClientViewportY) const;

	/**
	 * Checks if there are no obstacles on that position
	 *	\param blockFloor counts the ground tile as an obstacle
	 *	\returns The result if there is an obstacle or not
	 */
	bool isTileClear(uint16_t x, uint16_t y, uint8_t z, bool blockFloor = false) const;

	/**
	 * Checks if path is clear from fromPos to toPos
	 * Notice: This only checks a straight line if the path is clear, for path
	 *finding use getPathTo. \param fromPos from Source point \param toPos
	 *Destination point \param sameFloor checks if the destination is on same
	 *floor \returns The result if there is no obstacles
	 */
	bool isSightClear(const Position& fromPos, const Position& toPos, bool sameFloor = false) const;
	bool checkSightLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t z) const;

	const Tile* canWalkTo(const Creature& creature, const Position& pos) const;

	bool getPathMatching(const Creature& creature, std::vector<Direction>& dirList,
	                     const FrozenPathingConditionCall& pathCondition, const FindPathParams& fpp) const;

	std::unordered_map<std::string, Position> waypoints;

	QTreeLeafNode* getQTNode(uint16_t x, uint16_t y)
	{
		return QTreeNode::getLeafStatic<QTreeLeafNode*, QTreeNode*>(&root, x, y);
	}

	Spawns spawns;
	Towns towns;
	Houses houses;

	void getSpectatorsInternal(SpectatorVec& spectators, const Position& centerPos, int32_t minRangeX,
	                           int32_t maxRangeX, int32_t minRangeY, int32_t maxRangeY, int32_t minRangeZ,
	                           int32_t maxRangeZ, bool onlyPlayers = false) const;

private:
	SpectatorCache spectatorCache;
	SpectatorCache playersSpectatorCache;

	QTreeNode root;

	std::filesystem::path spawnfile;
	std::filesystem::path housefile;

	uint32_t width = 0;
	uint32_t height = 0;

	// Grid-based spatial partitioning for players
	std::unordered_map<std::pair<int32_t, int32_t>, std::vector<Player*>, pair_hash> playerGrid;

	friend class Game;
	friend class IOMap;
};

#endif // FS_MAP_H
