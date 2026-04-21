#pragma once

#include <SDL3/SDL.h>

#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

namespace Tiles {
enum {
  NONE,
  GRASS,
  DIRT,
  BRIDGE1,
  BRIDGE2,
  BRIDGE3,
  TREE1,
  TREE2,
  TREE3,
  COIN,
  ENEMY,
  SKY_PEACH,
  CLOUD_PEACH,
};
}

struct Map {
  static constexpr float TILE_SIZE = 16.0f;

 private:
  std::vector<uint16_t> tiles;
  size_t rows = 0;
  size_t cols = 0;

 public:
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
        if (rows == 0) cols = c;
        else if (c > cols) {
          SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                   "Could not parse tilemap from disk, the number of columns for all rows should be the same", nullptr);
          return false;
        }
        tiles.push_back(static_cast<uint16_t>(std::stoul(cell)));
      }
    }

    return true;
  }


  [[nodiscard]] size_t getRows() const { return rows; }
  [[nodiscard]] size_t getCols() const { return cols; }
  [[nodiscard]] std::vector<uint16_t> getTiles() const {
    return tiles;
  }
};
