#include "GameScene.h"

#include <sys/stat.h>

#include <cstdint>

#include "DynTile.h"
#include "Map.h"

// Initialise the player entity: set up animation frames, physics values
// (speed, jump velocity, acceleration), starting position, and collision box.
void GameScene::createPlayer() {
  constexpr size_t PLAYER_RUN_FRAMES = 16;
  constexpr uint16_t PLAYER_SIZE = 32;

  // Reserve space for all 5 animations: idle, run, jump, slide, death.
  std::vector<Frames> playerAnims;
  playerAnims.resize(5);

  // Idle: 4 frames, 0.15s per frame, texture coords on row 0 of the spritesheet.
  playerAnims[PlayerAnim::idle] = Frames(
      4, 0.15f,
      std::vector<glm::vec2>{
          glm::vec2(0.0f, 0.0f), glm::vec2(1 * PLAYER_SIZE, 0.0f),
          glm::vec2(2 * PLAYER_SIZE, 0.0f), glm::vec2(3 * PLAYER_SIZE, 0.0f)},
      PLAYER_SIZE, PLAYER_SIZE);

  // Run: 16 frames split across two rows (row 2 for first 8, row 3 for last 8).
  std::vector<glm::vec2> playerRunTexCoords{PLAYER_RUN_FRAMES};
  for (size_t i = 0; i < PLAYER_RUN_FRAMES; i++) {
    if (i <= 7) {
      playerRunTexCoords[i].x = static_cast<float>(i) * PLAYER_SIZE;
      playerRunTexCoords[i].y = 2 * PLAYER_SIZE;
    } else {
      playerRunTexCoords[i].x = static_cast<float>(i - 8) * PLAYER_SIZE;
      playerRunTexCoords[i].y = 3 * PLAYER_SIZE;
    }
  }
  playerAnims[PlayerAnim::run] = Frames(
      PLAYER_RUN_FRAMES, 0.1f, playerRunTexCoords, PLAYER_SIZE, PLAYER_SIZE);

  // Jump and slide: single-frame animations (just one texture coordinate).
  playerAnims[PlayerAnim::jump] = Frames(
      glm::vec2(2 * PLAYER_SIZE, 5 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::slide] = Frames(
      glm::vec2(2 * PLAYER_SIZE, 2 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);

  // Death: 4 frames, 0.3s per frame, on row 7. Non-looping (plays once).
  playerAnims[PlayerAnim::death] = Frames(
      4, 0.3f,
      std::vector<glm::vec2>{glm::vec2(0.0f, 7 * PLAYER_SIZE),
                             glm::vec2(1 * PLAYER_SIZE, 7 * PLAYER_SIZE),
                             glm::vec2(2 * PLAYER_SIZE, 7 * PLAYER_SIZE),
                             glm::vec2(3 * PLAYER_SIZE, 7 * PLAYER_SIZE)},
      PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::death].loop = false;

  // Position the player at the top-left of the level, 3 tiles above the bottom.
  player.pos = glm::vec2(0, SDLState::logicalHeight - 3 * PLAYER_SIZE);
  player.tex = resourceManager.getPlayerTex();
  // Physics values: horizontal max speed, vertical max speed, dash speed,
  // jump velocity (negative = upward), and horizontal acceleration.
  player.maxSpeed = glm::vec2(130.0f, 350.0f);
  player.dashSpeed = 200.0f;
  player.jumpVel = -350.0f;
  player.w = PLAYER_SIZE;
  player.h = static_cast<float>(PLAYER_SIZE);
  player.accel = glm::vec2(300, 0);
  player.anims = playerAnims;
  player.currAnim = PlayerAnim::idle;
  // Collision box is smaller than the sprite (centred, 8x10 px out of 32x32).
  player.collider.x = 12.0f;
  player.collider.y = 18.0f;
  player.collider.w = 8.0f;
  player.collider.h = 10.0f;
}

// Bake both background layers (bg1 and bg2) into render-target textures so
// they can be drawn efficiently each frame with parallax scrolling.
// Rather than re-drawing individual tiles every frame, we render them once
// to an off-screen texture at init time. At draw time, the pre-baked texture
// is blitted with an offset that scrolls at a different rate than the camera
// (parallax effect).
void GameScene::createBg() {
  // --- Layer 1: far background (sky and clouds) ---
  // This layer scrolls more slowly than the camera (small parallax factor),
  // creating the illusion of depth.
  bgTex1 = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapBgLayer1.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapBgLayer1.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(bgTex1, SDL_SCALEMODE_PIXELART);
  // Redirect all subsequent draw calls to bgTex1 instead of the screen.
  SDL_SetRenderTarget(sdlState.renderer, bgTex1);

  // `mapBgLayer1` refers to the first background layer of the level map that
  // defines what the background should look like.
  for (size_t r = 0; r < mapBgLayer1.getRows(); r++) {
    for (size_t c = 0; c < mapBgLayer1.getCols(); c++) {
      SDL_FRect src{.x = 0,
                    .y = 0,
                    .w = static_cast<float>(Map::TILE_SIZE),
                    .h = static_cast<float>(Map::TILE_SIZE)};
      const SDL_FRect dst{.x = static_cast<float>(c * Map::TILE_SIZE),
                          .y = static_cast<float>(r * Map::TILE_SIZE),
                          .w = static_cast<float>(Map::TILE_SIZE),
                          .h = static_cast<float>(Map::TILE_SIZE)};

      switch (mapBgLayer1.getTiles()[r * mapBgLayer1.getCols() + c]) {
        case Tiles::SKY_PEACH: {
          src.x = Map::TILE_SIZE;
          src.y = 11 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_PEACH: {
          src.x = Map::TILE_SIZE;
          src.y = 12 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_YELLOW: {
          src.x = Map::TILE_SIZE;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_YELLOW: {
          src.x = Map::TILE_SIZE;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_ORANGE: {
          src.x = Map::TILE_SIZE;
          src.y = 15 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_ORANGE: {
          src.x = Map::TILE_SIZE;
          src.y = 16 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_SKIN: {
          src.x = Map::TILE_SIZE;
          src.y = 9 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_SKIN: {
          src.x = Map::TILE_SIZE;
          src.y = 10 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_WHITE: {
          src.x = 0.0f;
          src.y = 9 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_WHITE: {
          src.x = 0.0f;
          src.y = 10 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_LIGHT_BLUE: {
          src.x = 0.0f;
          src.y = 11 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_LIGHT_BLUE: {
          src.x = 0.0f;
          src.y = 12 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_MEDIUM_BLUE: {
          src.x = 0.0f;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_MEDIUM_BLUE: {
          src.x = 0.0f;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_DARK_BLUE: {
          src.x = 0.0f;
          src.y = 15 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_LIGHT_SALMON: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 9 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_LIGHT_SALMON: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 10 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_DARK_SALMON: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 11 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_DARK_SALMON: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 12 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_LIGHT_PURPLE: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_LIGHT_PURPLE: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_DARK_PURPLE: {
          src.x = 2 * Map::TILE_SIZE;
          src.y = 15 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_LIGHT_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 9 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_LIGHT_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 10 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_MEDIUM_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 11 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_MEDIUM_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 12 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_DARK_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_DARK_SILVER: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_GRAY: {
          src.x = 3 * Map::TILE_SIZE;
          src.y = 15 * Map::TILE_SIZE;
          break;
        }
        case Tiles::NONE:
          continue;
        default:
          SDL_ShowSimpleMessageBox(
              SDL_MESSAGEBOX_ERROR, "Error",
              fmt::format("Unreachable: unhandled tile type of {} for "
                          "background layer 1",
                          mapBgLayer1.getTiles()[r * mapBgLayer1.getCols() + c])
                  .data(),
              nullptr);
          exit(1);
      }
      SDL_RenderTexture(sdlState.renderer, resourceManager.getWorldTex(), &src,
                        &dst);
    }
  }
  // Restore rendering to the screen (stop baking into bgTex1).
  SDL_SetRenderTarget(sdlState.renderer, nullptr);

  // --- Layer 2: midground (trees, bushes, decorative elements) ---
  // This layer scrolls at the same rate as the camera (parallax factor of -1),
  // giving the impression that it is closer to the player than bg layer 1.
  bgTex2 = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapBgLayer2.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapBgLayer2.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(bgTex2, SDL_SCALEMODE_PIXELART);
  SDL_SetRenderTarget(sdlState.renderer, bgTex2);

  // `mapBgLayer2` refers to the second background layer of the level map that
  // defines what the background should look like.
  for (size_t r = 0; r < mapBgLayer2.getRows(); r++) {
    for (size_t c = 0; c < mapBgLayer2.getCols(); c++) {
      SDL_FRect src{.x = 0,
                    .y = 0,
                    .w = static_cast<float>(Map::TILE_SIZE),
                    .h = static_cast<float>(Map::TILE_SIZE)};
      const SDL_FRect dst{.x = static_cast<float>(c * Map::TILE_SIZE),
                          .y = static_cast<float>(r * Map::TILE_SIZE),
                          .w = static_cast<float>(Map::TILE_SIZE),
                          .h = static_cast<float>(Map::TILE_SIZE)};

      switch (mapBgLayer2.getTiles()[r * mapBgLayer2.getCols() + c]) {
        case Tiles::TREE_CANOPY: {
          src.x = 0.0f;
          src.y = 3 * Map::TILE_SIZE;
          break;
        }
        case Tiles::TREE_MID: {
          src.x = 0.0f;
          src.y = 4 * Map::TILE_SIZE;
          break;
        }
        case Tiles::TREE_BASE: {
          src.x = 0.0f;
          src.y = 5 * Map::TILE_SIZE;
          break;
        }
        case Tiles::BUSH1: {
          src.x = 1 * Map::TILE_SIZE;
          src.y = 3 * Map::TILE_SIZE;
          break;
        }
        case Tiles::BUSH2: {
          src.x = 1 * Map::TILE_SIZE;
          src.y = 4 * Map::TILE_SIZE;
          break;
        }
        case Tiles::BUSH3: {
          src.x = 1 * Map::TILE_SIZE;
          src.y = 5 * Map::TILE_SIZE;
          break;
        }
        case Tiles::FLOWER_BUSH: {
          src.x = 1 * Map::TILE_SIZE;
          src.y = 6 * Map::TILE_SIZE;
          break;
        }
        case Tiles::YELLOW_WAVES: {
          src.x = 4 * Map::TILE_SIZE;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::YELLOW_WATER: {
          src.x = 4 * Map::TILE_SIZE;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::NONE:
          continue;
        default:
          SDL_ShowSimpleMessageBox(
              SDL_MESSAGEBOX_ERROR, "Error",
              fmt::format("Unreachable: unhandled tile type of {} for "
                          "background layer 2",
                          mapBgLayer2.getTiles()[r * mapBgLayer2.getCols() + c])
                  .data(),
              nullptr);
          exit(1);
      }
      SDL_RenderTexture(sdlState.renderer, resourceManager.getWorldTex(), &src,
                        &dst);
    }
  }
  SDL_SetRenderTarget(sdlState.renderer, nullptr);
}

// Bake the foreground layer (water, waves) into a render-target texture with
// a slight yellow tint and alpha transparency for a subtle overlay effect.
// The foreground is drawn on top of everything else (entities, player, etc.)
// to create the illusion that the player is moving behind water or similar
// environmental elements.
void GameScene::createFg() {
  // Create an off-screen texture to bake the foreground tiles into.
  fgTex = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapFgLayer.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapFgLayer.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(fgTex, SDL_SCALEMODE_PIXELART);
  // Apply a warm yellow tint and partial transparency so the gameplay layer
  // is still visible underneath.
  SDL_SetTextureColorMod(fgTex, 255, 255, 200);
  SDL_SetTextureAlphaMod(fgTex, 200);
  SDL_SetRenderTarget(sdlState.renderer, fgTex);

  // `mapFgLayer` refers to the first background layer of the level map that
  // defines what the background should look like.
  for (size_t r = 0; r < mapFgLayer.getRows(); r++) {
    for (size_t c = 0; c < mapFgLayer.getCols(); c++) {
      SDL_FRect src{.x = 0,
                    .y = 0,
                    .w = static_cast<float>(Map::TILE_SIZE),
                    .h = static_cast<float>(Map::TILE_SIZE)};
      const SDL_FRect dst{.x = static_cast<float>(c * Map::TILE_SIZE),
                          .y = static_cast<float>(r * Map::TILE_SIZE),
                          .w = static_cast<float>(Map::TILE_SIZE),
                          .h = static_cast<float>(Map::TILE_SIZE)};

      switch (mapFgLayer.getTiles()[r * mapFgLayer.getCols() + c]) {
        case Tiles::YELLOW_WAVES: {
          src.x = 4 * Map::TILE_SIZE;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::YELLOW_WATER: {
          src.x = 4 * Map::TILE_SIZE;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::NONE:
          continue;
        default:
          SDL_ShowSimpleMessageBox(
              SDL_MESSAGEBOX_ERROR, "Error",
              fmt::format("Unreachable: unhandled tile type of {} for "
                          "foreground layer",
                          mapFgLayer.getTiles()[r * mapFgLayer.getCols() + c])
                  .data(),
              nullptr);
          exit(1);
      }
      SDL_RenderTexture(sdlState.renderer, resourceManager.getWorldTex(), &src,
                        &dst);
    }
  }
  SDL_SetRenderTarget(sdlState.renderer, nullptr);
}

// Spawn all gameplay entities from the mid-layer tile map: static tiles
// (ground, dirt, bridges, boxes), moving platforms, coins, slimes, and the
// flagpost at their mapped positions.
//
// The map is stored as a 2D grid of Tiles enum values. We iterate row by row,
// column by column, and instantiate the appropriate C++ object for each tile.
// The Y coordinate is flipped: row 0 in the map is the top of the logical
// screen, so we compute the SDL Y coordinate as:
//   y = logicalHeight - (rows - r) * TILE_SIZE
// which places row 0 at the top and the last row at the bottom.
void GameScene::createEntities() {
  staticTiles.reserve(1000);
  dynTiles.reserve(10);
  coins.reserve(100);

  // `mapMidLayer` refers to the layer of the level map that defines the
  // player interactable tiles.
  for (size_t r = 0; r < mapMidLayer.getRows(); r++) {
    for (size_t c = 0; c < mapMidLayer.getCols(); c++) {
      // Each tile type maps to a specific game entity. The spritesheet
      // coordinate (src rect on the world texture) and collision properties
      // are set according to the tile's visual and functional role.
      switch (mapMidLayer.getTiles()[r * mapMidLayer.getCols() + c]) {
        case Tiles::GRASS: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{
              Frames(glm::vec2(0, 0), Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::DIRT1: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{Frames(
              glm::vec2(0, Map::TILE_SIZE), Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::DIRT2: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{Frames(
              glm::vec2(Map::TILE_SIZE, 0), Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::DIRT3: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{
              Frames(glm::vec2(Map::TILE_SIZE, Map::TILE_SIZE), Map::TILE_SIZE,
                     Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        // Moving platform: oscillates horizontally from its origin, carrying
        // the player when they stand on top. The collision box is shorter
        // than the sprite (h - 7) so the player can jump through from below.
        case Tiles::MOVING_PLATFORM_GRASS: {
          DynTile dynTile{};
          dynTile.origin =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          dynTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          dynTile.vel.x = 50.0f;
          dynTile.dir = 1;
          dynTile.tex = resourceManager.getPlatformTex();
          dynTile.w = static_cast<float>(Map::TILE_SIZE);
          dynTile.h = static_cast<float>(Map::TILE_SIZE);
          dynTile.collider.x = 0;
          dynTile.collider.y = 0;
          dynTile.collider.w = dynTile.w;
          dynTile.collider.h = dynTile.h - 7;
          dynTile.anims = std::vector<Frames>{
              Frames(glm::vec2(0, 0), Map::TILE_SIZE, Map::TILE_SIZE)};
          dynTiles.push_back(dynTile);
          break;
        }
        // Collectible coin: spinning animation, small collision box so the
        // pickup feels generous. Collected coins are tallied in collectedCoins.
        case Tiles::COIN: {
          Coin coin{};
          coin.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          coin.tex = resourceManager.getCoinTex();
          coin.w = static_cast<float>(Map::TILE_SIZE);
          coin.h = static_cast<float>(Map::TILE_SIZE);
          coin.collider.x = 5.0f;
          coin.collider.y = 5.0f;
          coin.collider.w = 6.0f;
          coin.collider.h = 6.0f;

          constexpr size_t COIN_ANIM_FRAMES = 12;
          std::vector<glm::vec2> coinTexCoords{COIN_ANIM_FRAMES};
          for (size_t i = 0; i < COIN_ANIM_FRAMES; i++) {
            coinTexCoords[i].x = static_cast<float>(i) * Map::TILE_SIZE;
            coinTexCoords[i].y = 0;
          }
          coin.anims = {Frames(COIN_ANIM_FRAMES, 0.1f, coinTexCoords,
                               Map::TILE_SIZE, Map::TILE_SIZE)};
          coin.currAnim = 0;

          coins.push_back(coin);
          break;
        }
        case Tiles::BRIDGE1: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0.0f;
          staticTile.collider.y = 0.0f;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h - 8;
          staticTile.anims =
              std::vector<Frames>{Frames(glm::vec2(9 * Map::TILE_SIZE, 0.0f),
                                         Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::BRIDGE2: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0.0f;
          staticTile.collider.y = 0.0f;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h - 8;
          staticTile.anims =
              std::vector<Frames>{Frames(glm::vec2(10 * Map::TILE_SIZE, 0.0f),
                                         Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::BRIDGE3: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0.0f;
          staticTile.collider.y = 0.0f;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h - 8;
          staticTile.anims =
              std::vector<Frames>{Frames(glm::vec2(11 * Map::TILE_SIZE, 0.0f),
                                         Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::BOX: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        SDLState::logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = static_cast<float>(Map::TILE_SIZE);
          staticTile.h = static_cast<float>(Map::TILE_SIZE);
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{
              Frames(glm::vec2(7 * Map::TILE_SIZE, 3 * Map::TILE_SIZE),
                     Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        // Enemy slime: patrols left/right, changes direction at walls and
        // edges. Player can stomp it from above to kill it.
        case Tiles::ENEMY: {
          Slime slime{};
          slime.w = 24.0f;
          slime.h = 16.0f;
          // NOTE: Subtracting by 4 pixels allows the slime tile to be
          // perfectly aligned with other tiles horizontally.
          slime.pos =
              glm::vec2(c * Map::TILE_SIZE - 4,
                        static_cast<float>(SDLState::logicalHeight -
                                           (mapMidLayer.getRows() - r - 1) *
                                               Map::TILE_SIZE) -
                            slime.h);
          slime.vel = glm::vec2(60.0f, 0.0f);
          slime.dir = 1;
          slime.tex = resourceManager.getSlimeTex();
          slime.collider.x = 8.0f;
          slime.collider.y = 5.0f;
          slime.collider.w = 8.0f;
          slime.collider.h = 10.0f;

          constexpr size_t SLIME_ANIM_FRAMES = 4;
          std::vector<glm::vec2> slimeTexCoords{};
          slimeTexCoords.resize(SLIME_ANIM_FRAMES);
          for (size_t i = 0; i < SLIME_ANIM_FRAMES; i++) {
            slimeTexCoords[i].x = static_cast<float>(i) * slime.w;
            slimeTexCoords[i].y = 32.0f;
          }
          slime.anims = {Frames(SLIME_ANIM_FRAMES, 0.1f, slimeTexCoords,
                                static_cast<uint16_t>(slime.w),
                                static_cast<uint16_t>(slime.h))};
          slime.currAnim = 0;

          slimes.push_back(slime);
          break;
        }
        // Flagpost: the level-end goal. When the player overlaps it while
        // grounded, shouldLevelComplete is set and the SceneManager transitions
        // to the end/victory scene.
        case Tiles::FLAGPOST: {
          flagPost.w = 60.0f;
          flagPost.h = 58.0f;
          flagPost.pos =
              glm::vec2(c * Map::TILE_SIZE - 1,
                        static_cast<float>(SDLState::logicalHeight -
                                           (mapMidLayer.getRows() - r - 1) *
                                               Map::TILE_SIZE) -
                            flagPost.h);
          flagPost.tex = resourceManager.getFlagPostTex();
          SDL_SetTextureColorMod(flagPost.tex, 255, 155, 0);
          constexpr size_t FLAGPOST_ANIM_FRAMES = 5;
          std::vector<glm::vec2> flagPostTexCoords{};
          flagPostTexCoords.resize(FLAGPOST_ANIM_FRAMES);
          for (size_t i = 0; i < FLAGPOST_ANIM_FRAMES; i++) {
            flagPostTexCoords[i].x = static_cast<float>(i) * flagPost.w;
            flagPostTexCoords[i].y = 0.0f;
          }
          flagPost.anims = {Frames(FLAGPOST_ANIM_FRAMES, 0.1f,
                                   flagPostTexCoords,
                                   static_cast<uint16_t>(flagPost.w),
                                   static_cast<uint16_t>(flagPost.h))};
          flagPost.currAnim = 0;
          break;
        }
        case Tiles::NONE:
          continue;
        default:
          SDL_ShowSimpleMessageBox(
              SDL_MESSAGEBOX_ERROR, "Error",
              fmt::format(
                  "Unreachable: unhandled tile type of {} for middle layer",
                  mapMidLayer.getTiles()[r * mapMidLayer.getCols() + c])
                  .data(),
              nullptr);
          exit(1);
      }
    }
  }
}

// Advance the game scene by one frame: step animations, update entities
// (dynTiles, coins, slimes, player), check for death (fall off screen) and
// level completion (reach the flagpost).
void GameScene::update(float dt) {
  // Clamp the delta time so that a large frame spike (e.g. from a pause or
  // stutter) doesn't cause physics objects to tunnel through walls.
  dt = glm::min(dt, maxPhysicsDt);

  // --- Update moving platforms ---
  // DynTiles oscillate horizontally or vertically between their origin and
  // a set range, carrying the player if they stand on top.
  for (auto &dynTile : dynTiles) {
    dynTile.update(dt, cam);
  }

  // --- Update coins ---
  // Advance the coin spinning animation and update the HUD text showing the
  // current coin count (incremented by Player::update on overlap).
  for (auto &coin : coins) {
    coin.anims[coin.currAnim].step(dt);
  }
  coinText.assign(fmt::format("Coins: {}", collectedCoins));

  // --- Update slimes ---
  // Advance each slime's walk animation, then let the slime AI move it
  // (patrol back and forth, change direction at walls/edges).
  for (auto &slime : slimes) {
    slime.anims[slime.currAnim].step(dt);
    slime.update(staticTiles, dt, cam);
  }
  slimesText.assign(fmt::format("Slain enemies: {}", slainSlimes));

  // --- Update dash cooldown HUD text ---
  // Show "Dashing!" while the dash is active, a countdown while it recharges,
  // and "Dash ready!" when available.
  if (player.dashDuration.isStarted() && !player.dashDuration.isTimeOut()) {
    dashCooldownText.assign("Dashing!");
  } else if (player.dashCooldown.isStarted() &&
             !player.dashCooldown.isTimeOut()) {
    float remaining =
        player.dashCooldown.getLen() - player.dashCooldown.getTime();
    dashCooldownText.assign(fmt::format("Dash in active: {:.1f}s", remaining));
  } else {
    dashCooldownText.assign("Dash ready!");
  }

  // --- Update flagpost animation ---
  flagPost.anims[flagPost.currAnim].step(dt);

  // --- Update player animation ---
  // Only advance the animation frame timer if the current animation actually
  // has multiple frames (single-frame anims like jump/slide have len = 0).
  if (player.anims[player.currAnim].getLen() != 0) {
    player.anims[player.currAnim].step(dt);
  }
  // Run full player physics: input handling, movement, gravity, collision
  // against static tiles and moving platforms, coin/slime overlap detection.
  player.update(
      sdlState, cam, static_cast<float>(mapMidLayer.getCols() * Map::TILE_SIZE),
      staticTiles, dynTiles, coins, collectedCoins, slimes, slainSlimes, dt);

  // --- Death check ---
  // If the player falls below the visible screen area (i.e. into a pit),
  // force the death animation.
  if (player.pos.y >= SDLState::logicalHeight) {
    player.currAnim = PlayerAnim::death;
  }

  // --- Level completion check ---
  // When the player reaches the flagpost while grounded and alive, freeze
  // the player in the idle pose and signal that the level is done. The
  // SceneManager will read shouldLevelComplete and start a transition.
  if (!shouldLevelComplete && player.pos.x >= flagPost.pos.x &&
      player.grounded && player.currAnim != PlayerAnim::death) {
    player.currAnim = PlayerAnim::idle;
    player.anims[player.currAnim].reset();
    shouldLevelComplete = true;
  }
}

// Render the entire scene: parallax background layers, static tiles, moving
// platforms, coins, slimes, HUD text, flagpost, player, and foreground overlay.
// Objects are drawn back-to-front to respect the depth ordering.
void GameScene::draw() {
  // --- Layer 1: far background (sky/clouds) ---
  // Uses a parallax factor of -0.3: scrolls at 30% of the camera speed,
  // creating the illusion of being far in the distance.
  constexpr float parallaxFactor[] = {-0.3f, -0.4f};

  const SDL_FRect bgTex1Dst = {
      .x = parallaxFactor[0] * cam.x,
      .y = -Map::TILE_SIZE,
      .w = static_cast<float>(mapBgLayer1.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapBgLayer1.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, bgTex1, nullptr, &bgTex1Dst);

  // --- Layer 2: midground (trees, bushes) ---
  // Scrolls at 100% of the camera speed (factor of -1.0 on x), matching
  // the movement of the gameplay layer so it feels grounded in the world.
  const SDL_FRect bgTex2Dst = {
      .x = -cam.x,
      .y = SDLState::logicalHeight -
           static_cast<float>(mapBgLayer2.getRows() * Map::TILE_SIZE) - cam.y,
      .w = static_cast<float>(mapBgLayer2.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapBgLayer2.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, bgTex2, nullptr, &bgTex2Dst);

  // --- Gameplay layer: static tiles (ground, platforms, boxes) ---
  for (auto &staticTile : staticTiles) {
    staticTile.draw(sdlState, cam);
  }
  // --- Moving platforms ---
  for (auto &dynTile : dynTiles) {
    dynTile.draw(sdlState, cam);
  }
  // --- Coins ---
  for (auto &coin : coins) {
    coin.draw(sdlState, cam);
  }
  // --- Enemies (slimes) ---
  for (auto &slime : slimes) {
    slime.draw(sdlState, cam);
  }

  // --- HUD (heads-up display) ---
  // Drawn in screen space (not affected by camera) so they stay fixed.
  coinText.draw();
  slimesText.draw();
  dashCooldownText.draw();

  // --- Flagpost (level-end goal) ---
  flagPost.draw(sdlState, cam);

  // --- Player (drawn last so it appears on top of everything) ---
  player.draw(sdlState, cam);

  // --- Foreground overlay (water/waves) ---
  // Drawn on top of everything with partial transparency, creating the
  // illusion that the player is behind an environmental element.
  const SDL_FRect fgTexDst = {
      .x = -cam.x,
      .y = SDLState::logicalHeight -
           static_cast<float>(mapFgLayer.getRows() * Map::TILE_SIZE) - cam.y,
      .w = static_cast<float>(mapFgLayer.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapFgLayer.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, fgTex, nullptr, &fgTexDst);
}
