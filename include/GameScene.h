#pragma once

#include <SDL3/SDL.h>

#include "Coin.h"
#include "DynTile.h"
#include "Enemy.h"
#include "Map.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"
#include "Text.h"

class GameScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Map mapBgLayer{};
  Map mapMidLayer{};
  bool debug = false;
  Player player{};
  SDL_Texture *bgTex{nullptr};
  std::vector<Enemy> enemies{};
  std::vector<StaticTile> staticTiles{};
  std::vector<DynTile> dynTiles{};
  std::vector<Coin> coins{};
  SDL_FRect cam{};
  Text coinText{};
  size_t collectedCoins;

 private:
  void createPlayer();
  void createBg();
  void createEntities();

  void init() {
    if (!mapBgLayer.parse("./assets/levels/1/bg.csv")) {
      exit(1);
    }
    if (!mapMidLayer.parse("./assets/levels/1/mid.csv")) {
      exit(1);
    }

    createBg();
    createEntities();
    createPlayer();

    cam.w = SDLState::logicalWidth;
    cam.h = SDLState::logicalHeight;
    cam.x = 0;
    cam.y = 0;

    coinText = Text(sdlState, fmt::format("Coins: {}", collectedCoins),
                    glm::vec2(10, 10));
  }

 public:
  GameScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        collectedCoins(0) {
    init();
  }

  void reset() {
    mapBgLayer = Map();
    mapMidLayer = Map();
    player = Player();
    enemies.clear();
    staticTiles.clear();
    dynTiles.clear();
    coins.clear();
    collectedCoins = 0;
    init();
  }

  void update(float dt);
  void draw();

  // NOTE: We are forced to define an explictit `free()`, because static `Text`
  // instances may be created by `Game` class and they need to be freed manually
  // before `main()` ends. `SDL_ttf` requires a strict order of
  // deinitialization.
  // TODO: Implement `TextManager`, so static `Text` instances do not need to be
  // created.
  ~GameScene() {
    SDL_DestroyTexture(bgTex);
    coinText.free();
  }
};
