#pragma once

#include <SDL3/SDL.h>
#include <fmt/base.h>

#include "Coin.h"
#include "Colors.h"
#include "DynTile.h"
#include "FlagPost.h"
#include "Map.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "Slime.h"
#include "StaticTile.h"
#include "Text.h"

// The main gameplay scene: owns all map layers, entities, the camera, and the
// HUD. This is where the actual platforming takes place. The scene is fully
// reset on retry or when returning to the start menu.
struct GameScene {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;

  // Maximum delta time for physics updates (≈60 FPS). If a frame takes longer
  // than 17ms (e.g. due to a lag spike), the game will run multiple fixed-size
  // physics steps to keep the simulation stable. This prevents entities from
  // tunneling through walls at low framerates.
  static constexpr float maxPhysicsDt = 0.017f;

  // Four tilemap layers for parallax scrolling:
  //   bgLayer1 — farthest background (sky, mountains)
  //   bgLayer2 — clouds / mid-background decorations
  //   midLayer — trees, bridges, foreground decorations (scrolls with camera)
  //   fgLayer  — gameplay layer: ground, platforms, coins, enemies
  Map mapBgLayer1;
  Map mapBgLayer2;
  Map mapMidLayer;
  Map mapFgLayer;

  Player player;
  FlagPost flagPost;
  SDL_Texture *bgTex1;  // Rendered texture for bgLayer1
  SDL_Texture *bgTex2;  // Rendered texture for bgLayer2
  SDL_Texture *fgTex;   // Rendered texture for fgLayer
  std::vector<Slime> slimes;
  std::vector<StaticTile> staticTiles;  // Collidable ground tiles
  std::vector<DynTile> dynTiles;        // Moving platforms
  std::vector<Coin> coins;

  // Camera defines the visible portion of the world. Centered on the player
  // with smoothing. w/h match the logical resolution (320×180). Only entities
  // intersecting the camera rect are drawn (frustum culling).
  SDL_FRect cam;
  size_t collectedCoins;
  size_t slainSlimes;
  bool shouldLevelComplete = false;

  // HUD text rendered in screen space (not affected by camera).
  Text coinText;
  Text slimesText;
  Text dashCooldownText;

 private:
  // Build the player entity: configure sprite animations, physics values,
  // and starting position from the map data.
  void createPlayer();
  // Bake background tile layers (bg1, bg2) into off-screen textures for
  // parallax rendering. Each layer is drawn at a different camera scale.
  void createBg();
  // Bake the foreground gameplay layer into an off-screen texture. This
  // layer scrolls with the camera and contains terrain visuals.
  void createFg();
  // Spawn all dynamic entities (coins, slimes, moving platforms) from
  // tile-map metadata. Static ground tiles are also collected here.
  void createEntities();

  // Load all four CSV tilemaps, then bake background/foreground textures and
  // spawn entities. Called once at construction and again on reset().
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
  // Construct the full game scene: load map CSVs, bake textures, spawn
  // entities and the player. Also sets up the HUD text objects.
  GameScene(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        bgTex1(nullptr),
        bgTex2(nullptr),
        fgTex(nullptr),
        cam(0.0f),
        collectedCoins(0),
        slainSlimes(0),
        coinText(sdlState, fmt::format("Coins: {}", collectedCoins),
                 glm::vec2(5, 5)),
        slimesText(sdlState, fmt::format("Slain enemies: {}", slainSlimes),
                   glm::vec2(5, 15)),
        dashCooldownText(sdlState, "", glm::vec2(5, 25)) {
    init();
    coinText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
    slimesText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b, Colors::fg.a);
    dashCooldownText.setColor(Colors::fg.r, Colors::fg.g, Colors::fg.b,
                              Colors::fg.a);
  }

  // Fully re-initialize the scene from scratch. Destroys and recreates maps,
  // entities, camera, and HUD counters. Called when the player retries or
  // returns to the start menu.
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
    slainSlimes = 0;
    shouldLevelComplete = false;
    init();
  }

  // Advance the game world by dt seconds: player input & physics, entity
  // AI, collision detection/resolution, animation stepping, and camera
  // follow. Uses fixed-size sub-steps for stable simulation.
  void update(float dt);
  // Render the visible game world: parallax backgrounds, foreground
  // terrain, all entities (player, coins, slimes), and the HUD overlay.
  void draw();

  // Clean up GPU-side background textures created during baking.
  ~GameScene() {
    SDL_DestroyTexture(bgTex1);
    SDL_DestroyTexture(bgTex2);
  }
};
