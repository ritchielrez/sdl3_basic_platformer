#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Tile type identifiers used in the CSV level files. Each value matches an
// index in the world_tileset.png sprite sheet (0 = empty/transparent).
// The naming convention groups tiles visually: e.g. SKY_* / CLOUD_* pairs
// are background sky gradients with matching cloud decorations, TREE_* forms
// a multi-tile tree, and BRIDGE* spans gaps over water.
namespace Tiles {
enum {
  NONE,   // 0 = empty cell, not rendered
  GRASS,  // Solid ground tile (grass on top)
  // Underground dirt variations
  DIRT1,
  DIRT2,
  DIRT3,
  MOVING_PLATFORM_GRASS,  // Moving platform (see DynTile)
  COIN,                   // Collectible coin (see Coin)
  ENEMY,                  // Enemy spawn point (see Slime)
  // Different types of tiles for background layer 1 that make up the sky and
  // the clouds
  SKY_PEACH,
  CLOUD_PEACH,
  SKY_YELLOW,
  CLOUD_YELLOW,
  SKY_ORANGE,
  CLOUD_ORANGE,
  SKY_SKIN,
  CLOUD_SKIN,
  SKY_WHITE,
  CLOUD_WHITE,
  SKY_LIGHT_BLUE,
  CLOUD_LIGHT_BLUE,
  SKY_MEDIUM_BLUE,
  CLOUD_MEDIUM_BLUE,
  SKY_DARK_BLUE,
  SKY_LIGHT_SALMON,
  CLOUD_LIGHT_SALMON,
  SKY_DARK_SALMON,
  CLOUD_DARK_SALMON,
  SKY_LIGHT_PURPLE,
  CLOUD_LIGHT_PURPLE,
  SKY_DARK_PURPLE,
  SKY_LIGHT_SILVER,
  CLOUD_LIGHT_SILVER,
  SKY_MEDIUM_SILVER,
  CLOUD_MEDIUM_SILVER,
  SKY_DARK_SILVER,
  CLOUD_DARK_SILVER,
  SKY_GRAY,
  TREE_CANOPY,  // Tree foliage (midground parallax)
  TREE_MID,     // Tree trunk middle
  TREE_BASE,    // Tree trunk base
  BUSH1,        // Decorative bushes
  BUSH2,
  BUSH3,
  FLOWER_BUSH,
  YELLOW_WAVES,  // Water surface
  YELLOW_WATER,  // Deep water tiles
  BRIDGE1,       // Wooden bridge sections (midground)
  BRIDGE2,
  BRIDGE3,
  BOX,  // Collidable crate
};
}

// A tile map loaded from a CSV file. The grid is stored in row-major order in
// a flat vector for cache-friendly iteration. Each cell value corresponds to a
// Tiles enum entry and maps to a 16×16 pixel region in the world tileset.
struct Map {
  // Each tile is 16×16 pixels at the game's logical resolution.
  static constexpr uint16_t TILE_SIZE = 16;

 private:
  // Flat array of tile IDs in row-major order: index = row * cols + col.
  std::vector<uint16_t> tiles;
  size_t rows = 0;
  size_t cols = 0;

 public:
  // Parse a comma-separated CSV file into the tile grid. The first row
  // determines the expected column count; subsequent rows are validated
  // against it. Empty lines are skipped. An error dialog is shown if the
  // file cannot be opened or has inconsistent row lengths.
  bool parse(const std::string& filePath) {
    std::ifstream input{filePath};
    tiles.reserve(10000);

    if (!input.is_open()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not open tilemap from disk", nullptr);
      exit(1);
    }
    std::string cell, line;
    for (; std::getline(input, line); rows++) {
      if (line.empty()) continue;
      std::istringstream ss(std::move(line));

      for (size_t c = 1; std::getline(ss, cell, ','); c++) {
        if (rows == 0)
          cols = c;
        else if (c > cols) {
          SDL_ShowSimpleMessageBox(
              SDL_MESSAGEBOX_ERROR, "Error",
              "Could not parse tilemap from disk, the number of columns for "
              "all rows should be the same",
              nullptr);
          return false;
        }
        tiles.push_back(static_cast<uint16_t>(std::stoul(cell)));
      }
    }

    return true;
  }

  [[nodiscard]] size_t getRows() const { return rows; }
  [[nodiscard]] size_t getCols() const { return cols; }
  [[nodiscard]] std::vector<uint16_t> getTiles() const { return tiles; }
};
