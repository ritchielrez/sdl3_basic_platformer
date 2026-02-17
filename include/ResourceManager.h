#include <SDL3/SDL.h>
#include <stb_image.h>

#include <cstdlib>

class ResourceManager {
  SDL_Texture *playerTex;
  SDL_Texture *worldTex;
  SDL_Texture *platformsTex;

 public:
  ResourceManager()
      : playerTex(nullptr), worldTex(nullptr), platformsTex(nullptr) {}
  ResourceManager(SDLState &sdlState) {
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
  }

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
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

    SDL_DestroySurface(surface);
    stbi_image_free(pixData);

    return tex;
  }

  SDL_Texture *getPlayerTex() const { return playerTex; }
  SDL_Texture *getWorldTex() const { return worldTex; }
  SDL_Texture *getPlatformTex() const { return platformsTex; }

  ~ResourceManager() { SDL_DestroyTexture(playerTex); }
};
