#pragma once

#include <SDL3/SDL.h>
#include <stb_image.h>

#include <cstdlib>
#include <string_view>

#include "SDL3/SDL_surface.h"
#include "SDLState.h"

// `ResourceManager` is a class that, you guessed it right manages resources.
// Here resources refer to textures, which are basically images that are loaded
// onto the GPU VRAM.
class ResourceManager {
  // Here is a list of textures the game needs.
  SDL_Texture *coinTex;
  SDL_Texture *playerTex;
  SDL_Texture *worldTex;
  SDL_Texture *platformsTex;
  SDL_Texture *enemyTex;

 public:
  // No argument constructor setting everything to the default value of
  // `nullptr`. This does not create any textures.
  ResourceManager()
      : coinTex(nullptr),
        playerTex(nullptr),
        worldTex(nullptr),
        platformsTex(nullptr),
        enemyTex(nullptr) {}
  // Parameterized constructor to initialize everything properly. This does
  // create the necessary textures.
  ResourceManager(SDLState &sdlState) {
    // Create all of these textures from image files, if any of them failed to
    // create then exit early.
    coinTex = loadTex(sdlState, "assets/sprites/coin.png");
    if (!coinTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Coin texture could not be loaded", nullptr);
      exit(1);
    }

    playerTex = loadTex(sdlState, "assets/sprites/knight.png");
    if (!playerTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Player texture could not be loaded", nullptr);
      exit(1);
    }

    worldTex = loadTex(sdlState, "assets/sprites/world_tileset.png");
    if (!worldTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "World texture could not be loaded", nullptr);
      exit(1);
    }

    platformsTex = loadTex(sdlState, "assets/sprites/platforms.png");
    if (!platformsTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Platforms texture could not be loaded",
                               nullptr);
      exit(1);
    }

    enemyTex = loadTex(sdlState, "assets/sprites/slime_green.png");
    if (!enemyTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Enemy texture could not be loaded", nullptr);
      exit(1);
    }
  }

  // Delete copy and move constructors and copy and move assignment operators.
  // This way, a `ResourceManager` object cannot be copied or moved into a new
  // `ResourceManager` object.
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;
  ResourceManager(ResourceManager &&) noexcept = delete;
  ResourceManager &operator=(ResourceManager &&) noexcept = delete;

  SDL_Texture *loadTex(const SDLState &sdlState,
                       const std::string_view &filePath) {
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *pixData =
        stbi_load(filePath.data(), &width, &height, &channels, 4);

    // NOTE: SDL_Surface and SDL_Texture are pretty much the same, except
    // SDL_Surface stores texture data in RAM whereas SDL_Texture stores texture
    // data in GPU VRAM.
    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        width, height, SDL_PIXELFORMAT_RGBA32, pixData, width * 4);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlState.renderer, surface);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_PIXELART);

    SDL_DestroySurface(surface);
    stbi_image_free(pixData);

    return tex;
  }

  [[nodiscard]] SDL_Texture *getCoinTex() const { return coinTex; }
  [[nodiscard]] SDL_Texture *getPlayerTex() const { return playerTex; }
  [[nodiscard]] SDL_Texture *getWorldTex() const { return worldTex; }
  [[nodiscard]] SDL_Texture *getPlatformTex() const { return platformsTex; }
  [[nodiscard]] SDL_Texture *getEnemyTex() const { return enemyTex; }

  ~ResourceManager() {
    SDL_DestroyTexture(coinTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(worldTex);
    SDL_DestroyTexture(platformsTex);
    SDL_DestroyTexture(enemyTex);
  }
};
