#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vector>

#include "DynTile.h"
#include "Player.h"
#include "StaticTile.h"

namespace Tiles {

enum Tiles {
  GRASS = 1,
  DIRT,
  BRIDGE1,
  BRIDGE2,
  BRIDGE3,
  TREE1,
  TREE2,
  TREE3,
};

}

// bool Game::debug;
// std::array<uint16_t, Game::MAP_COLS * Game::MAP_ROWS> map{0};
// Player Game::player;
// std::vector<StaticTile> Game::staticTiles;
// std::vector<DynTile> Game::dynTiles;

struct Game {
  static constexpr float TILE_SIZE = 16.0f;
  static constexpr size_t MAP_ROWS = 5;
  static constexpr size_t MAP_COLS = 50;
  static inline bool debug = false;
  static inline uint16_t map[MAP_ROWS][MAP_COLS];
  static inline Player player{};
  static inline std::vector<StaticTile> staticTiles{};
  static inline std::vector<DynTile> dynTiles{};
};
