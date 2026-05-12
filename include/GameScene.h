#pragma once

#include <SDL3/SDL.h>
#include <fmt/base.h>

#include "Coin.h"
#include "DynTile.h"
#include "Enemy.h"
#include "Map.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"
#include "Text.h"

struct GameScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  Map mapBgLayer;
  Map mapMidLayer;
  Player player{};
  SDL_Texture *bgTex;
  std::vector<Enemy> enemies;
  std::vector<StaticTile> staticTiles;
  std::vector<DynTile> dynTiles;
  std::vector<Coin> coins;
  SDL_FRect cam;
  size_t collectedCoins;
  Text coinText;

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
  }

 public:
  GameScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        bgTex(nullptr),
        cam(0.0f),
        collectedCoins(0),
        coinText(sdlState, fmt::format("Coins: {}", collectedCoins),
                 glm::vec2(10, 10)) {
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
  ~GameScene() { SDL_DestroyTexture(bgTex); }
};
