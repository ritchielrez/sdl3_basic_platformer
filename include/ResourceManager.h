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

  // This is a custom method that creates a texture from an image.
  SDL_Texture *loadTex(const SDLState &sdlState,
                       const std::string_view &filePath) {
    // These values are needed by stb_image in order to load pixel data from any
    // image.
    int width = 0;     // width of the picture
    int height = 0;    // height of the picture
    int channels = 0;  // number of values used to represent one pixel (3 for
                       // RGB, 4 for RGBA)
    // Use stb_image to load the pixel color data of the image. stb_image is a
    // very well-known imager loading library used by many games and game
    // engines.
    stbi_uc *pixData =
        stbi_load(filePath.data(), &width, &height, &channels, 4);

    // Create a SDL_Surface from the pixel data of the image, because you cannot
    // create a SDL_Texture directly from the pixel data. NOTE: SDL_Surface and
    // SDL_Texture are pretty much the same, except SDL_Surface stores texture
    // data in RAM whereas SDL_Texture stores texture data in GPU VRAM.
    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        width, height, SDL_PIXELFORMAT_RGBA32, pixData, width * 4);
    // Create a SDL_Texture from a SDL_Surface.
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlState.renderer, surface);
    // Set the texture scaling mode to pixel art so when the textures are
    // upscaled they do not get blurry.
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_PIXELART);

    // The SDL_Surface is not needed anymore, it is an intermediate resource
    // used to create a texture.
    SDL_DestroySurface(surface);
    stbi_image_free(pixData);

    // Return the created texture.
    return tex;
  }

  // Getter methods for the textures. [[nodiscard]] ensures that the return
  // value of the getter cannot be unused by the caller.
  [[nodiscard]] SDL_Texture *getCoinTex() const { return coinTex; }
  [[nodiscard]] SDL_Texture *getPlayerTex() const { return playerTex; }
  [[nodiscard]] SDL_Texture *getWorldTex() const { return worldTex; }
  [[nodiscard]] SDL_Texture *getPlatformTex() const { return platformsTex; }
  [[nodiscard]] SDL_Texture *getEnemyTex() const { return enemyTex; }

  // Destructor to deallocate all textures. This prevents any memory
  // leaks.
  ~ResourceManager() {
    SDL_DestroyTexture(coinTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(worldTex);
    SDL_DestroyTexture(platformsTex);
    SDL_DestroyTexture(enemyTex);
  }
};
