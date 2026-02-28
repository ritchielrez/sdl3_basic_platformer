#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vector>

#include "DynTile.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"

namespace Tiles {
enum {
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

struct Game {
  static constexpr float TILE_SIZE = 16.0f;
  static constexpr size_t MAP_ROWS = 5;
  static constexpr size_t MAP_COLS = 50;
  static inline bool debug = false;
  static inline uint16_t map[MAP_ROWS][MAP_COLS];
  static inline Player player{};
  static inline std::vector<StaticTile> staticTiles{};
  static inline std::vector<DynTile> dynTiles{};

  static inline void createPlayer(const SDLState &sdlState,
                                  const ResourceManager &resourceManager) {
    constexpr size_t PLAYER_IDLE_FRAMES = 4;
    constexpr size_t PLAYER_RUN_FRAMES = 16;
    constexpr float PLAYER_SIZE = 32.0f;

    std::vector<Frames> playerAnims;
    playerAnims.resize(5);

    playerAnims[PlayerAnim::idle] = Frames(
        PLAYER_IDLE_FRAMES, 0.5f,
        std::vector<glm::vec2>{
            glm::vec2(0 * PLAYER_SIZE, 0.0f), glm::vec2(1 * PLAYER_SIZE, 0.0f),
            glm::vec2(2 * PLAYER_SIZE, 0.0f), glm::vec2(3 * PLAYER_SIZE, 0.0f)},
        PLAYER_SIZE, PLAYER_SIZE);

    std::vector<glm::vec2> playerRunTexCoords{PLAYER_RUN_FRAMES};
    for (size_t i = 0; i < PLAYER_RUN_FRAMES; i++) {
      if (i <= 7) {
        playerRunTexCoords[i].x = i * PLAYER_SIZE;
        playerRunTexCoords[i].y = 2 * PLAYER_SIZE;
      } else {
        playerRunTexCoords[i].x = (i - 8) * PLAYER_SIZE;
        playerRunTexCoords[i].y = 3 * PLAYER_SIZE;
      }
    }
    playerAnims[PlayerAnim::run] =
        Frames(16, 2.0f, playerRunTexCoords, PLAYER_SIZE, PLAYER_SIZE);
    playerAnims[PlayerAnim::jump] = Frames(
        glm::vec2(2 * PLAYER_SIZE, 5 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);
    playerAnims[PlayerAnim::slide] = Frames(
        glm::vec2(2 * PLAYER_SIZE, 2 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);

    player.pos = glm::vec2(0, sdlState.logHeight - 64);
    player.tex = resourceManager.getPlayerTex();
    player.maxSpeedX = 100.0f;
    player.jumpVel = -125.0f;
    player.w = PLAYER_SIZE;
    player.h = PLAYER_SIZE;
    player.accel = glm::vec2(300, 0);
    player.anims = playerAnims;
    player.currAnim = PlayerAnim::idle;
    player.collider.x = 12.0f;
    player.collider.y = 18.0f;
    player.collider.w = 8.0f;
    player.collider.h = 10.0f;
  }

  static inline void loadTileMap(const SDLState &sdlState,
                                 const ResourceManager &resourceManager) {
    for (uint16_t r = 1; r <= 2; r++) {
      for (uint16_t c = 0; c < MAP_COLS; c++) {
        map[MAP_ROWS - r][c] = Tiles::DIRT;
      }
    }

    staticTiles.reserve(100);
    dynTiles.reserve(100);

    for (uint16_t r = 0; r < MAP_ROWS; r++) {
      for (uint16_t c = 0; c < MAP_COLS; c++) {
        switch (map[r][c]) {
          case Tiles::DIRT: {
            StaticTile staticTile{};
            staticTile.pos = glm::vec2(
                c * TILE_SIZE, sdlState.logHeight - (MAP_ROWS - r) * TILE_SIZE);
            staticTile.tex = resourceManager.getWorldTex();
            staticTile.w = TILE_SIZE;
            staticTile.h = TILE_SIZE;
            staticTile.collider.x = 0;
            staticTile.collider.y = 0;
            staticTile.collider.w = staticTile.w;
            staticTile.collider.h = staticTile.h;
            staticTile.anims = std::vector<Frames>{
                Frames(glm::vec2(0, TILE_SIZE), TILE_SIZE, TILE_SIZE)};
            staticTiles.push_back(staticTile);
            break;
          }
          case 0:
            break;
          default:
            assert(false && "Unreachable");
        }
      }
    }
  }
};
