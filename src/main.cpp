#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <array>
#include <cassert>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

#include "DynTile.h"
#include "Game.h"
#include "Log.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StaticTile.h"
#include "defer.h"

#define FONT_HEIGHT 8.0f
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

void createPlayer(const SDLState &, const ResourceManager &);
void loadTileMap(const SDLState &, const ResourceManager &);

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3", nullptr);
    return 1;
  }
  defer(SDL_Quit());

  SDLState sdlState{"learn_sdl3", SDL_WINDOW_RESIZABLE, nullptr};
  ResourceManager resourceManager{sdlState};

  createPlayer(sdlState, resourceManager);
  loadTileMap(sdlState, resourceManager);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();

  ImGui_ImplSDL3_InitForSDLRenderer(sdlState.win, sdlState.renderer);
  ImGui_ImplSDLRenderer3_Init(sdlState.renderer);
  defer(ImGui_ImplSDLRenderer3_Shutdown(); ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext(););

  io.Fonts->AddFontFromFileTTF("assets/fonts/PixelOperator8.ttf");

  uint64_t prevTime = SDL_GetTicks();
  bool running = true;
  while (running) {
    uint64_t nowTime = SDL_GetTicks();
    float dt = (nowTime - prevTime) / 1000.0f;

    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      switch (event.type) {
        case SDL_EVENT_QUIT: {
          running = false;
          break;
        }
        case SDL_EVENT_WINDOW_RESIZED: {
          sdlState.winWidth = event.window.data1;
          sdlState.winHeight = event.window.data2;
          break;
        }
        // NOTE: Keyboard events are better if you do not want multiple key
        // down events in a single frame. With events, there is a delay
        // between consecutive key down events.
        case SDL_EVENT_KEY_DOWN: {
          if (event.key.scancode == SDL_SCANCODE_F1) Game::debug = !Game::debug;
          Game::player.handleKeyInput(sdlState, event.key.scancode, true);
          break;
        }
        case SDL_EVENT_KEY_UP: {
          Game::player.handleKeyInput(sdlState, event.key.scancode, false);
          break;
        }
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // This ensures the dockspace does not cover the entire screen, so the
    // actual game can seen behind the windows.
    ImGui::DockSpaceOverViewport(0, nullptr,
                                 ImGuiDockNodeFlags_PassthruCentralNode);

    // ImGui::Begin("My Window");
    // if (ImGui::Button("Button1")) {
    //   Log::debug("Button1 pressed\n");
    // }
    // ImGui::End();

    ImGui::Render();

    // Clear screen
    SDL_SetRenderDrawColor(sdlState.renderer, 20, 152, 220, 255);
    SDL_RenderClear(sdlState.renderer);

    // Update

    // Only animate the player if the current animation has multiple frames.
    // If it has one frame, the timer length/duration is set to 0.
    if (Game::player.anims[Game::player.currAnim].getLen() != 0) {
      Game::player.anims[Game::player.currAnim].step(dt);
    }
    Game::player.update(sdlState, dt);

    // Render
    Game::player.draw(sdlState);
    for (auto &staticTile : Game::staticTiles) {
      staticTile.draw(sdlState);
    }
    for (auto &dynTile : Game::dynTiles) {
      dynTile.draw(sdlState);
    }

    SDL_SetRenderLogicalPresentation(sdlState.renderer, 0, 0,
                                     SDL_LOGICAL_PRESENTATION_DISABLED);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          sdlState.renderer);
    SDL_SetRenderLogicalPresentation(sdlState.renderer, sdlState.logWidth,
                                     sdlState.logHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // Swap buffers
    SDL_RenderPresent(sdlState.renderer);

    prevTime = nowTime;
  }

  return 0;
}

void createPlayer(const SDLState &sdlState,
                  const ResourceManager &resourceManager) {
  constexpr size_t PLAYER_IDLE_FRAMES = 4;
  constexpr size_t PLAYER_RUN_FRAMES = 16;
  constexpr float PLAYER_SIZE = 32.0f;

  std::vector<Frames> playerAnims;
  playerAnims.resize(5);

  playerAnims[PlayerAnim::idle] = Frames(
      PLAYER_IDLE_FRAMES, 0.5f,
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
  playerAnims[PlayerAnim::run] =
      Frames(16, 2.0f, playerRunTexCoords, PLAYER_SIZE, PLAYER_SIZE);
  playerAnims[PlayerAnim::jump] = Frames(
      glm::vec2(2 * PLAYER_SIZE, 5 * PLAYER_SIZE), PLAYER_SIZE, PLAYER_SIZE);

  Game::player.pos = glm::vec2(0, sdlState.logHeight - 64);
  Game::player.tex = resourceManager.getPlayerTex();
  Game::player.maxSpeedX = 100.0f;
  Game::player.jumpAccel = -125.0f;
  Game::player.w = PLAYER_SIZE;
  Game::player.h = PLAYER_SIZE;
  Game::player.accel = glm::vec2(300, 0);
  Game::player.anims = playerAnims;
  Game::player.currAnim = PlayerAnim::idle;
  Game::player.collider.x = 12.0f;
  Game::player.collider.y = 18.0f;
  Game::player.collider.w = 8.0f;
  Game::player.collider.h = 10.0f;
}

void loadTileMap(const SDLState &sdlState,
                 const ResourceManager &resourceManager) {
  for (uint16_t r = 1; r <= 2; r++) {
    for (uint16_t c = 0; c < Game::MAP_COLS; c++) {
      Game::map[Game::MAP_ROWS - r][c] = Tiles::DIRT;
    }
  }

  Game::staticTiles.reserve(100);
  Game::dynTiles.reserve(100);

  for (uint16_t r = 0; r < Game::MAP_ROWS; r++) {
    for (uint16_t c = 0; c < Game::MAP_COLS; c++) {
      switch (Game::map[r][c]) {
        case Tiles::DIRT: {
          StaticTile staticTile{};
          staticTile.pos = glm::vec2(
              c * Game::TILE_SIZE,
              sdlState.logHeight - (Game::MAP_ROWS - r) * Game::TILE_SIZE);
          staticTile.tex = resourceManager.getWorldTex();
          staticTile.w = Game::TILE_SIZE;
          staticTile.h = Game::TILE_SIZE;
          staticTile.collider.x = 0;
          staticTile.collider.y = 0;
          staticTile.collider.w = staticTile.w;
          staticTile.collider.h = staticTile.h;
          staticTile.anims = std::vector<Frames>{Frames(
              glm::vec2(0, Game::TILE_SIZE), Game::TILE_SIZE, Game::TILE_SIZE)};
          Game::staticTiles.push_back(staticTile);
          break;
        }
        case 0:
          break;
        default:
          assert(false && "Unreachable");
      }
    }
  }
}
