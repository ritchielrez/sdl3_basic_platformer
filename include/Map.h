#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
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
};
}

struct Map {
  static constexpr float TILE_SIZE = 16.0f;

 private:
  // std::array<std::array<uint16_t, COLS>, ROWS> tiles{Tiles::NONE};
  std::vector<std::vector<uint16_t>> tiles;
  size_t rows = 0;
  size_t cols = 0;

 public:
  void parse(const std::string& filePath) {
    std::ifstream input{filePath};

    if (!input.is_open()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not open map.csv file", nullptr);
      exit(1);
    }

    std::string line;
    while (std::getline(input, line)) {
      if (line.empty()) continue;
      std::istringstream ss(std::move(line));

      std::string cell;
      std::vector<uint16_t> row;
      while (std::getline(ss, cell, ',')) {
        row.push_back(std::move(static_cast<uint16_t>(std::stoul(std::move(cell)))));
      }
      tiles.push_back(std::move(row));
    }

    rows = tiles.size();
    cols = tiles[0].size();
  }

  const size_t getRows() const { return rows; }
  const size_t getCols() const { return cols; }
  const std::vector<std::vector<uint16_t>> getTiles() const {
    return tiles;
  }
};
