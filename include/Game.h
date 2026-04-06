#pragma once

#include <fmt/base.h>

#include <array>
#include <glm/glm.hpp>
#include <vector>

#include "Coin.h"
#include "DynTile.h"
#include "Enemy.h"
#include "Map.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"
#include "Text.h"

struct Game {
  static inline Map map{};
  static inline bool debug = false;
  static inline Player player{};
  static inline std::vector<Enemy> enemies{};
  static inline std::vector<StaticTile> staticTiles{};
  static inline std::vector<DynTile> dynTiles{};
  static inline std::vector<Coin> coins{};
  static inline SDL_FRect cam{};
  static inline Text coinText{};
  static inline size_t collectedCoins = 0;

 private:
  static inline void createPlayer(const SDLState &sdlState,
                                  const ResourceManager &resourceManager) {
    constexpr size_t PLAYER_IDLE_FRAMES = 4;
    constexpr size_t PLAYER_RUN_FRAMES = 16;
    constexpr float PLAYER_SIZE = 32.0f;

    std::vector<Frames> playerAnims;
    playerAnims.resize(5);

    playerAnims[PlayerAnim::idle] = Frames(
        PLAYER_IDLE_FRAMES, 0.15f,
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
    playerAnims[PlayerAnim::run] = Frames(
        PLAYER_RUN_FRAMES, 0.1f, playerRunTexCoords, PLAYER_SIZE, PLAYER_SIZE);
    playerAnims[PlayerAnim::jump] = Frames(
        glm::vec2(2 * PLAYER_SIZE, 5 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);
    playerAnims[PlayerAnim::slide] = Frames(
        glm::vec2(2 * PLAYER_SIZE, 2 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);

    player.pos = glm::vec2(0, sdlState.logicalHeight - 3 * PLAYER_SIZE);
    player.tex = resourceManager.getPlayerTex();
    player.maxSpeedX = 100.0f;
    player.jumpVel = -300.0f;
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
    map.parse("./assets/map.csv");

    staticTiles.reserve(100);
    dynTiles.reserve(100);
    coins.reserve(100);

    for (uint16_t r = 0; r < Map::ROWS; r++) {
      for (uint16_t c = 0; c < Map::COLS; c++) {
        switch (map.getTiles()[r][c]) {
          case Tiles::DIRT: {
            StaticTile staticTile{};
            staticTile.pos = glm::vec2(
                c * Map::TILE_SIZE,
                sdlState.logicalHeight - (Map::ROWS - r) * Map::TILE_SIZE);
            staticTile.tex = resourceManager.getWorldTex();
            staticTile.w = Map::TILE_SIZE;
            staticTile.h = Map::TILE_SIZE;
            staticTile.collider.x = 0;
            staticTile.collider.y = 0;
            staticTile.collider.w = staticTile.w;
            staticTile.collider.h = staticTile.h;
            staticTile.anims = std::vector<Frames>{Frames(
                glm::vec2(0, Map::TILE_SIZE), Map::TILE_SIZE, Map::TILE_SIZE)};
            staticTiles.push_back(staticTile);
            break;
          }
          case Tiles::COIN: {
            Coin coin{};
            coin.pos = glm::vec2(
                c * Map::TILE_SIZE,
                sdlState.logicalHeight - (Map::ROWS - r) * Map::TILE_SIZE);
            coin.tex = resourceManager.getCoinTex();
            coin.w = Map::TILE_SIZE;
            coin.h = Map::TILE_SIZE;
            coin.collider.x = 5.0f;
            coin.collider.y = 5.0f;
            coin.collider.w = 6.0f;
            coin.collider.h = 6.0f;

            constexpr size_t COIN_ANIM_FRAMES = 12;
            std::vector<glm::vec2> coinTexCoords{COIN_ANIM_FRAMES};
            for (size_t i = 0; i < COIN_ANIM_FRAMES; i++) {
              coinTexCoords[i].x = i * Map::TILE_SIZE;
              coinTexCoords[i].y = 0;
            }
            coin.anims = {Frames(COIN_ANIM_FRAMES, 0.1f, coinTexCoords,
                                 Map::TILE_SIZE, Map::TILE_SIZE)};
            coin.currAnim = 0;

            coins.push_back(coin);
            break;
          }
          case Tiles::ENEMY: {
            const float ENEMY_SIZE = 24.0f;
            Enemy enemy{};
            enemy.pos = glm::vec2(c * Map::TILE_SIZE,
                                  (sdlState.logicalHeight -
                                   (Map::ROWS - r - 1) * Map::TILE_SIZE) -
                                      ENEMY_SIZE);
            enemy.tex = resourceManager.getEnemyTex();
            enemy.w = ENEMY_SIZE;
            enemy.h = ENEMY_SIZE;
            enemy.collider.x = 0;
            enemy.collider.y = 0;
            enemy.collider.w = enemy.w;
            enemy.collider.h = enemy.h;

            constexpr size_t ENEMY_ANIM_FRAMES = 4;
            std::vector<glm::vec2> enemyTexCoords{ENEMY_ANIM_FRAMES};
            for (size_t i = 0; i < ENEMY_ANIM_FRAMES; i++) {
              enemyTexCoords[i].x = i * ENEMY_SIZE;
              enemyTexCoords[i].y = ENEMY_SIZE;
            }
            enemy.anims = {Frames(ENEMY_ANIM_FRAMES, 0.1f, enemyTexCoords,
                                  ENEMY_SIZE, ENEMY_SIZE)};
            enemy.currAnim = 0;

            enemies.push_back(enemy);
            break;
          };
          case Tiles::NONE:
            break;
          default:
            assert(false && "Unreachable");
        }
      }
    }
  }

 public:
  static inline void init(const SDLState &sdlState,
                          const ResourceManager &resourceManager) {
    createPlayer(sdlState, resourceManager);
    loadTileMap(sdlState, resourceManager);

    cam.w = sdlState.logicalWidth;
    cam.h = sdlState.logicalHeight;
    cam.x = 0;
    cam.y = 0;

    coinText = Text(sdlState, fmt::format("Coin: {}", collectedCoins),
                    glm::vec2(0, 0));
  }
  static inline void reset(const SDLState &sdlState,
                           const ResourceManager &resourceManager) {
    player = Player();
    staticTiles.clear();
    dynTiles.clear();
    coins.clear();
    collectedCoins = 0;
    init(sdlState, resourceManager);
  }
  // NOTE: We are forced to define an explictit `free()`, because static `Text`
  // instances may be created by `Game` class and they need to be freed manually
  // before `main()` ends. `SDL_ttf` requires a strict order of
  // deinitialization.
  // TODO: Implement `TextManager`, so static `Text` instances do not need to be
  // created.
  ~Game() { coinText.free(); }

  static inline void update(const SDLState &sdlState, float dt) {
    // Only animate the player if the current animation has multiple frames.
    // If it has one frame, the timer length/duration is set to 0.
    if (player.anims[player.currAnim].getLen() != 0) {
      player.anims[player.currAnim].step(dt);
    }
    player.update(sdlState, dt);

    for (auto &coin : coins) {
      coin.anims[coin.currAnim].step(dt);
    }
    coinText.assign(fmt::format("Coin: {}", collectedCoins));

    for (auto &enemy : enemies) {
      enemy.anims[enemy.currAnim].step(dt);
    }
  }

  static inline void draw(const SDLState &sdlState) {
    player.draw(sdlState);
    for (auto &staticTile : staticTiles) {
      staticTile.draw(sdlState);
    }
    for (auto &dynTile : dynTiles) {
      dynTile.draw(sdlState);
    }
    for (auto &coin : coins) {
      coin.draw(sdlState);
    }
    for (auto &enemy : enemies) {
      enemy.draw(sdlState);
    }

    coinText.draw();
  }
};
