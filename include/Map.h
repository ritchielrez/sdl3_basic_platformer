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
  static constexpr size_t ROWS = 5;
  static constexpr size_t COLS = 200;

 private:
  std::array<std::array<uint16_t, COLS>, ROWS> tiles{Tiles::NONE};

 public:
  void parse(const std::string& filePath) {
    std::ifstream input{filePath};

    if (!input.is_open()) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Could not open map.csv file", nullptr);
      exit(1);
    }

    std::string row;
    for (size_t i = 0; std::getline(input, row) && i <= ROWS; i++) {
      std::istringstream ss(std::move(row));

      std::string cell;
      for (size_t j = 0; std::getline(ss, cell, ',') && j <= COLS; j++) {
        tiles[i][j] = std::stoul(std::move(cell));
      }
    }
  }

  const std::array<std::array<uint16_t, COLS>, ROWS> getTiles() const {
    return tiles;
  }
};
