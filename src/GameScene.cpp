#include "GameScene.h"

#include <sys/stat.h>

#include <cstdint>

#include "DynTile.h"
#include "Map.h"

void GameScene::createPlayer() {
  constexpr size_t PLAYER_RUN_FRAMES = 16;
  constexpr uint16_t PLAYER_SIZE = 32;

  std::vector<Frames> playerAnims;
  playerAnims.resize(5);

  playerAnims[PlayerAnim::idle] = Frames(
      4, 0.15f,
      std::vector<glm::vec2>{
          glm::vec2(0.0f, 0.0f), glm::vec2(1 * PLAYER_SIZE, 0.0f),
          glm::vec2(2 * PLAYER_SIZE, 0.0f), glm::vec2(3 * PLAYER_SIZE, 0.0f)},
      PLAYER_SIZE, PLAYER_SIZE);

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
  playerAnims[PlayerAnim::jump] = Frames(
      glm::vec2(2 * PLAYER_SIZE, 5 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::slide] = Frames(
      glm::vec2(2 * PLAYER_SIZE, 2 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::death] = Frames(
      4, 0.3f,
      std::vector<glm::vec2>{glm::vec2(0.0f, 7 * PLAYER_SIZE),
                             glm::vec2(1 * PLAYER_SIZE, 7 * PLAYER_SIZE),
                             glm::vec2(2 * PLAYER_SIZE, 7 * PLAYER_SIZE),
                             glm::vec2(3 * PLAYER_SIZE, 7 * PLAYER_SIZE)},
      PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::death].loop = false;

  // player.pos = glm::vec2(0, SDLState::logicalHeight - 3 * PLAYER_SIZE);
  player.pos = glm::vec2(1827, 72);
  player.tex = resourceManager.getPlayerTex();
  player.maxSpeed = glm::vec2(130.0f, 350.0f);
  player.dashSpeed = 200.0f;
  player.jumpVel = -350.0f;
  player.w = PLAYER_SIZE;
  player.h = static_cast<float>(PLAYER_SIZE);
  player.accel = glm::vec2(300, 0);
  player.anims = playerAnims;
  player.currAnim = PlayerAnim::idle;
  player.collider.x = 12.0f;
  player.collider.y = 18.0f;
  player.collider.w = 8.0f;
  player.collider.h = 10.0f;
}

void GameScene::createBg() {
  bgTex1 = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapBgLayer1.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapBgLayer1.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(bgTex1, SDL_SCALEMODE_PIXELART);
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
  SDL_SetRenderTarget(sdlState.renderer, nullptr);

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

void GameScene::createFg() {
  fgTex = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapFgLayer.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapFgLayer.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(fgTex, SDL_SCALEMODE_PIXELART);
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

void GameScene::createEntities() {
  staticTiles.reserve(1000);
  dynTiles.reserve(10);
  coins.reserve(100);

  // `mapMidLayer` refers to the layer of the level map that defines the
  // player interactable tiles.
  for (size_t r = 0; r < mapMidLayer.getRows(); r++) {
    for (size_t c = 0; c < mapMidLayer.getCols(); c++) {
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

void GameScene::update(float dt) {
  dt = glm::min(dt, maxPhysicsDt);

  for (auto &dynTile : dynTiles) {
    dynTile.update(dt, cam);
  }

  for (auto &coin : coins) {
    coin.anims[coin.currAnim].step(dt);
  }
  coinText.assign(fmt::format("Coins: {}", collectedCoins));

  for (auto &slime : slimes) {
    slime.anims[slime.currAnim].step(dt);
    slime.update(staticTiles, dt, cam);
  }
  slimesText.assign(fmt::format("Slain enemies: {}", slainSlimes));

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

  flagPost.anims[flagPost.currAnim].step(dt);

  // Only animate the player if the current animation has multiple frames.
  // If it has one frame, the timer length/duration is set to 0.
  if (player.anims[player.currAnim].getLen() != 0) {
    player.anims[player.currAnim].step(dt);
  }
  player.update(
      sdlState, cam, static_cast<float>(mapMidLayer.getCols() * Map::TILE_SIZE),
      staticTiles, dynTiles, coins, collectedCoins, slimes, slainSlimes, dt);

  // If the player fall below the screen, they die.
  if (player.pos.y >= SDLState::logicalHeight) {
    player.currAnim = PlayerAnim::death;
  }

  // Check if the player reaches the flagpost, which indicates the level is
  // complete. Set the level complete flag and reset the player animation to
  // idle.
  if (!shouldLevelComplete && player.pos.x >= flagPost.pos.x &&
      player.grounded && player.currAnim != PlayerAnim::death) {
    player.currAnim = PlayerAnim::idle;
    player.anims[player.currAnim].reset();
    shouldLevelComplete = true;
  }
}

void GameScene::draw() {
  constexpr float parallaxFactor[] = {-0.3f, -0.4f};

  const SDL_FRect bgTex1Dst = {
      .x = parallaxFactor[0] * cam.x,
      .y = -Map::TILE_SIZE,
      .w = static_cast<float>(mapBgLayer1.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapBgLayer1.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, bgTex1, nullptr, &bgTex1Dst);

  const SDL_FRect bgTex2Dst = {
      .x = -cam.x,
      .y = SDLState::logicalHeight -
           static_cast<float>(mapBgLayer2.getRows() * Map::TILE_SIZE) - cam.y,
      .w = static_cast<float>(mapBgLayer2.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapBgLayer2.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, bgTex2, nullptr, &bgTex2Dst);

  for (auto &staticTile : staticTiles) {
    staticTile.draw(sdlState, cam);
  }
  for (auto &dynTile : dynTiles) {
    dynTile.draw(sdlState, cam);
  }
  for (auto &coin : coins) {
    coin.draw(sdlState, cam);
  }
  for (auto &slime : slimes) {
    slime.draw(sdlState, cam);
  }

  coinText.draw();
  slimesText.draw();
  dashCooldownText.draw();

  flagPost.draw(sdlState, cam);

  player.draw(sdlState, cam);

  const SDL_FRect fgTexDst = {
      .x = -cam.x,
      .y = SDLState::logicalHeight -
           static_cast<float>(mapFgLayer.getRows() * Map::TILE_SIZE) - cam.y,
      .w = static_cast<float>(mapFgLayer.getCols() * Map::TILE_SIZE),
      .h = static_cast<float>(mapFgLayer.getRows() * Map::TILE_SIZE)};
  SDL_RenderTexture(sdlState.renderer, fgTex, nullptr, &fgTexDst);
}
