#pragma once

#include <SDL3/SDL.h>
#include <stb_image.h>

#include <cstdlib>
#include <string_view>

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_surface.h"
#include "SDLState.h"

// Manages all textures (GPU-resident images) the game needs. Textures are
// loaded from PNG files via stb_image, uploaded to GPU VRAM through SDL, then
// cached for the lifetime of the game. The manager is non-copyable and
// non-movable so texture ownership is clear — destruction frees all GPU memory.
class ResourceManager {
  SDL_Texture *coinTex;
  SDL_Texture *playerTex;
  SDL_Texture *worldTex;
  SDL_Texture *platformsTex;
  SDL_Texture *slimeTex;
  SDL_Texture *startSceneBgTex;
  SDL_Texture *deathSceneBgTex;

 public:
  // Default constructor — all textures null. Exists so ResourceManager can
  // be declared before the real load happens.
  ResourceManager()
      : coinTex(nullptr),
        playerTex(nullptr),
        worldTex(nullptr),
        platformsTex(nullptr),
        slimeTex(nullptr),
        startSceneBgTex(nullptr) {}
  // Load all game textures from disk. Each PNG is decoded via stb_image into
  // CPU-side pixel data, converted to an SDL_Surface (system RAM), and then
  // uploaded to an SDL_Texture (GPU VRAM). If any texture fails to load the
  // game exits with an error — there is no graceful fallback for missing art.
  ResourceManager(SDLState &sdlState) {
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

    slimeTex = loadTex(sdlState, "assets/sprites/slime_green.png");
    if (!slimeTex) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                               "Slime texture could not be loaded", nullptr);
      exit(1);
    }

    startSceneBgTex = loadTex(sdlState, "assets/sprites/start_scene_bg.png");
    if (!startSceneBgTex) {
      SDL_ShowSimpleMessageBox(
          SDL_MESSAGEBOX_ERROR, "Error",
          "The background texture for start scene could not be loaded",
          nullptr);
      exit(1);
    }

    deathSceneBgTex = loadTex(sdlState, "assets/sprites/death_scene_bg.png");
    if (!deathSceneBgTex) {
      SDL_ShowSimpleMessageBox(
          SDL_MESSAGEBOX_ERROR, "Error",
          "The background texture for death scene could not be loaded",
          nullptr);
      exit(1);
    }
  }

  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;
  ResourceManager(ResourceManager &&) noexcept = delete;
  ResourceManager &operator=(ResourceManager &&) noexcept = delete;

  // Load a single texture from a PNG image file. The pipeline is:
  //   1. stbi_load() decodes PNG → raw RGBA pixel data in RAM.
  //   2. SDL_CreateSurfaceFrom() wraps that data in an SDL_Surface.
  //   3. SDL_CreateTextureFromSurface() uploads the surface to GPU VRAM.
  //   4. Texture scale mode is set to NEAREST so pixel-art upscaling remains
  //      crisp without bilinear filtering blur.
  SDL_Texture *loadTex(const SDLState &sdlState,
                       const std::string_view &filePath) {
    int width = 0;
    int height = 0;
    int channels = 0;
    // stb_image is a single-header C library for loading images. It handles
    // PNG, JPEG, BMP, GIF, and others. Requesting 4 channels forces RGBA
    // output regardless of the source format.
    stbi_uc *pixData =
        stbi_load(filePath.data(), &width, &height, &channels, 4);

    // SDL_Surface resides in system RAM and is the only way to create an
    // SDL_Texture from raw pixel data. The pitch (stride) is width × 4 bytes
    // since we use RGBA32 format (4 bytes per pixel).
    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        width, height, SDL_PIXELFORMAT_RGBA32, pixData, width * 4);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlState.renderer, surface);

    // Nearest-neighbor filtering: each pixel in the source maps to an integer
    // block of screen pixels. This preserves the hard edges of pixel art.
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

    SDL_DestroySurface(surface);
    stbi_image_free(pixData);

    return tex;
  }

  [[nodiscard]] SDL_Texture *getCoinTex() const { return coinTex; }
  [[nodiscard]] SDL_Texture *getPlayerTex() const { return playerTex; }
  [[nodiscard]] SDL_Texture *getWorldTex() const { return worldTex; }
  [[nodiscard]] SDL_Texture *getPlatformTex() const { return platformsTex; }
  [[nodiscard]] SDL_Texture *getSlimeTex() const { return slimeTex; }
  [[nodiscard]] SDL_Texture *getStartSceneBgTex() const {
    return startSceneBgTex;
  }
  [[nodiscard]] SDL_Texture *getDeathSceneBgTex() const {
    return deathSceneBgTex;
  }

  // Destructor releases all GPU memory. Each SDL_DestroyTexture decrements
  // the texture's internal reference count and frees the VRAM allocation.
  ~ResourceManager() {
    SDL_DestroyTexture(coinTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(worldTex);
    SDL_DestroyTexture(platformsTex);
    SDL_DestroyTexture(slimeTex);
    SDL_DestroyTexture(startSceneBgTex);
    SDL_DestroyTexture(deathSceneBgTex);
  }
};
