#include "GameScene.h"

#include "Map.h"

void GameScene::createPlayer() {
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
  player.maxSpeedX = 130.0f;
  player.jumpVel = -350.0f;
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

void GameScene::createBg() {
  bgTex = SDL_CreateTexture(
      sdlState.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      static_cast<int>(mapBgLayer.getCols() * Map::TILE_SIZE),
      static_cast<int>(mapBgLayer.getRows() * Map::TILE_SIZE));
  SDL_SetTextureScaleMode(bgTex, SDL_SCALEMODE_PIXELART);
  SDL_SetRenderTarget(sdlState.renderer, bgTex);

  // `mapBgLayer` refers to the background layer of the level map that defines
  // what the background should look like.
  for (size_t r = 0; r < mapBgLayer.getRows(); r++) {
    for (size_t c = 0; c < mapBgLayer.getCols(); c++) {
      SDL_FRect src{.x = 0, .y = 0, .w = Map::TILE_SIZE, .h = Map::TILE_SIZE};
      const SDL_FRect dst{.x = c * Map::TILE_SIZE,
                          .y = r * Map::TILE_SIZE,
                          .w = Map::TILE_SIZE,
                          .h = Map::TILE_SIZE};

      switch (mapBgLayer.getTiles()[r * mapBgLayer.getCols() + c]) {
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
          src.x = 0;
          src.y = 9 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_WHITE: {
          src.x = 0;
          src.y = 10 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_LIGHT_BLUE: {
          src.x = 0;
          src.y = 11 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_LIGHT_BLUE: {
          src.x = 0;
          src.y = 12 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_MEDIUM_BLUE: {
          src.x = 0;
          src.y = 13 * Map::TILE_SIZE;
          break;
        }
        case Tiles::CLOUD_MEDIUM_BLUE: {
          src.x = 0;
          src.y = 14 * Map::TILE_SIZE;
          break;
        }
        case Tiles::SKY_DARK_BLUE: {
          src.x = 0;
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
              fmt::format(
                  "Unreachable: unhandled tile type of {} for background layer",
                  mapMidLayer.getTiles()[r * mapMidLayer.getCols() + c])
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

  constexpr float ENEMY_SIZE = 24.0f;

  // `mapMidLayer` refers to the layer of the level map that defines the
  // player interactable tiles.
  for (size_t r = 0; r < mapMidLayer.getRows(); r++) {
    for (size_t c = 0; c < mapMidLayer.getCols(); c++) {
      switch (mapMidLayer.getTiles()[r * mapMidLayer.getCols() + c]) {
        case Tiles::GRASS: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = Map::TILE_SIZE;
          staticTile.h = Map::TILE_SIZE;
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
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
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
        case Tiles::DIRT2: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = Map::TILE_SIZE;
          staticTile.h = Map::TILE_SIZE;
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
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = Map::TILE_SIZE;
          staticTile.h = Map::TILE_SIZE;
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
        case Tiles::PLATFORM_GRASS: {
          StaticTile staticTile{};
          staticTile.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
          staticTile.tex = resourceManager.getPlatformTex();
          staticTile.w = Map::TILE_SIZE;
          staticTile.h = Map::TILE_SIZE;
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{
              Frames(glm::vec2(0, 0), Map::TILE_SIZE, Map::TILE_SIZE)};
          staticTiles.push_back(staticTile);
          break;
        }
        case Tiles::COIN: {
          Coin coin{};
          coin.pos =
              glm::vec2(c * Map::TILE_SIZE,
                        sdlState.logicalHeight -
                            (mapMidLayer.getRows() - r) * Map::TILE_SIZE);
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
          Enemy enemy{};
          // NOTE: Subtracting by 4 pixels allows the enemy tile to be
          // perfectly aligned with other tiles horizontally.
          enemy.pos =
              glm::vec2(c * Map::TILE_SIZE - 4,
                        (sdlState.logicalHeight -
                         (mapMidLayer.getRows() - r - 1) * Map::TILE_SIZE) -
                            ENEMY_SIZE);
          enemy.vel = glm::vec2(60.0f, 0.0f);
          enemy.dir = 1;
          enemy.tex = resourceManager.getEnemyTex();
          enemy.w = ENEMY_SIZE;
          enemy.h = ENEMY_SIZE;
          enemy.collider.x = 8.0f;
          enemy.collider.y = 13.0f;
          enemy.collider.w = enemy.w - 16;
          enemy.collider.h = enemy.h - 14;

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
  // Only animate the player if the current animation has multiple frames.
  // If it has one frame, the timer length/duration is set to 0.
  if (player.anims[player.currAnim].getLen() != 0) {
    player.anims[player.currAnim].step(dt);
  }
  player.update(sdlState, cam, staticTiles, dynTiles, coins, collectedCoins,
                enemies, dt);

  for (auto &coin : coins) {
    coin.anims[coin.currAnim].step(dt);
  }
  coinText.assign(fmt::format("Coins: {}", collectedCoins));

  for (auto &enemy : enemies) {
    enemy.anims[enemy.currAnim].step(dt);
    enemy.update(staticTiles, dt, cam);
  }

  // If the player fall 150 pixels below of the bottom of the screen, they
  // die.
  if (player.pos.y >= SDLState::logicalHeight + 150.0f) {
    player.death = true;
  }
}

void GameScene::draw() {
  constexpr float parallaxFactor = -0.3f;
  const SDL_FRect bgTexDst = {.x = parallaxFactor * cam.x,
                              .y = -Map::TILE_SIZE,
                              .w = mapBgLayer.getCols() * Map::TILE_SIZE,
                              .h = mapBgLayer.getRows() * Map::TILE_SIZE};
  SDL_RenderTexture(sdlState.renderer, bgTex, nullptr, &bgTexDst);

  for (auto &staticTile : staticTiles) {
    staticTile.draw(sdlState, cam);
  }
  for (auto &dynTile : dynTiles) {
    dynTile.draw(sdlState, cam);
  }
  for (auto &coin : coins) {
    coin.draw(sdlState, cam);
  }
  for (auto &enemy : enemies) {
    enemy.draw(sdlState, cam);
  }

  coinText.draw();

  player.draw(sdlState, cam);
}
