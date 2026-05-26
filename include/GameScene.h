#pragma once

#include <SDL3/SDL.h>
#include <fmt/base.h>

#include "Coin.h"
#include "DynTile.h"
#include "Map.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "Slime.h"
#include "StaticTile.h"
#include "Text.h"

struct GameScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  static constexpr float maxPhysicsDt = 0.017f;
  Map mapBgLayer1;
  Map mapBgLayer2;
  Map mapMidLayer;
  Map mapFgLayer;
  Player player{};
  SDL_Texture *bgTex1;
  SDL_Texture *bgTex2;
  SDL_Texture *fgTex;
  std::vector<Slime> slimes;
  std::vector<StaticTile> staticTiles;
  std::vector<DynTile> dynTiles;
  std::vector<Coin> coins;
  SDL_FRect cam;
  size_t collectedCoins;
  Text coinText;

 private:
  void createPlayer();
  void createBg();
  void createFg();
  void createEntities();

  void init() {
    if (!mapBgLayer1.parse("./assets/levels/1/bg1.csv")) {
      exit(1);
    }
    if (!mapBgLayer2.parse("./assets/levels/1/bg2.csv")) {
      exit(1);
    }
    if (!mapMidLayer.parse("./assets/levels/1/mid.csv")) {
      exit(1);
    }
    if (!mapFgLayer.parse("./assets/levels/1/fg.csv")) {
      exit(1);
    }

    createBg();
    createFg();
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
        bgTex1(nullptr),
        bgTex2(nullptr),
        fgTex(nullptr),
        cam(0.0f),
        collectedCoins(0),
        coinText(sdlState, fmt::format("Coins: {}", collectedCoins),
                 glm::vec2(10, 10)) {
    init();
  }

  void reset() {
    mapBgLayer1 = Map();
    mapBgLayer2 = Map();
    mapMidLayer = Map();
    mapFgLayer = Map();
    player = Player();
    slimes.clear();
    staticTiles.clear();
    dynTiles.clear();
    coins.clear();
    collectedCoins = 0;
    init();
  }

  void update(float dt);
  void draw();

  ~GameScene() {
    SDL_DestroyTexture(bgTex1);
    SDL_DestroyTexture(bgTex2);
  }
};
