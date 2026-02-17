#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <microui.h>

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

mu_Context mu_ctx{0};

TTF_Font *ttf_font = nullptr;
TTF_TextEngine *ttf_engine = nullptr;

const char btn_map[]{MU_MOUSE_LEFT, MU_MOUSE_MIDDLE, MU_MOUSE_RIGHT};

void createPlayer(const SDLState &, const ResourceManager &);
void loadTileMap(const SDLState &, const ResourceManager &);

int text_width(mu_Font mu_font, const char *str, int len) {
  if (len == -1) {
    len = 0;
  }
  int width = 0;
  if (TTF_MeasureString(ttf_font, str, len, 0, &width, nullptr)) {
    return width;
  }
  return 0;
}

#define FONT_HEIGHT 8.0f

int text_height(mu_Font mu_font) { return static_cast<int>(FONT_HEIGHT); }

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3", nullptr);
    return 1;
  }
  defer(SDL_Quit());

  if (!TTF_Init()) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not initialize SDL3_ttf", nullptr);
    return 1;
  }

  ttf_font = TTF_OpenFont("assets/fonts/PixelOperator8.ttf", FONT_HEIGHT);
  if (!ttf_font) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not open font", nullptr);
    return 1;
  }

  SDLState sdlState{"learn_sdl3", SDL_WINDOW_RESIZABLE, nullptr};
  ResourceManager resourceManager{sdlState};

  ttf_engine = TTF_CreateRendererTextEngine(sdlState.renderer);
  if (!ttf_engine) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                             "Could not create text engine", nullptr);
    return 1;
  }

  createPlayer(sdlState, resourceManager);
  loadTileMap(sdlState, resourceManager);

  mu_init(&mu_ctx);
  mu_ctx.text_width = text_width;
  mu_ctx.text_height = text_height;

  uint64_t prevTime = SDL_GetTicks();
  bool running = true;
  while (running) {
    uint64_t nowTime = SDL_GetTicks();
    float dt = (nowTime - prevTime) / 1000.0f;

    SDL_Event event{0};
    while (SDL_PollEvent(&event)) {
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
        case SDL_EVENT_MOUSE_MOTION: {
          if (!SDL_ConvertEventToRenderCoordinates(sdlState.renderer, &event)) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR, "Error",
                "Could not convert event to render coordinates", nullptr);
            return 1;
          }
          mu_input_mousemove(&mu_ctx, static_cast<int>(event.motion.x),
                             static_cast<int>(event.motion.y));
          break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
          if (!SDL_ConvertEventToRenderCoordinates(sdlState.renderer, &event)) {
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_ERROR, "Error",
                "Could not convert event to render coordinates", nullptr);
            return 1;
          }
          const char b = btn_map[event.button.button - 1];
          if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            mu_input_mousedown(&mu_ctx, static_cast<int>(event.button.x),
                               static_cast<int>(event.button.y), b);
          if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            mu_input_mouseup(&mu_ctx, static_cast<int>(event.button.x),
                             static_cast<int>(event.button.y), b);
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

    mu_begin(&mu_ctx);
    if (mu_begin_window(&mu_ctx, "My Window", mu_rect(10, 10, 140, 86))) {
      int rows[]{60, -1};
      mu_layout_row(&mu_ctx, 2, rows, 0);

      mu_label(&mu_ctx, "First:");
      if (mu_button(&mu_ctx, "Button1")) {
        Log::debug("Button1 pressed\n");
      }

      mu_label(&mu_ctx, "Second:");
      if (mu_button(&mu_ctx, "Button2")) {
        mu_open_popup(&mu_ctx, "My Popup");
      }

      if (mu_begin_popup(&mu_ctx, "My Popup")) {
        mu_label(&mu_ctx, "Hello world!");
        mu_end_popup(&mu_ctx);
      }

      mu_end_window(&mu_ctx);
    }
    mu_end(&mu_ctx);

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

    mu_Command *mu_cmd = nullptr;
    while (mu_next_command(&mu_ctx, &mu_cmd)) {
      switch (mu_cmd->type) {
        case MU_COMMAND_TEXT: {
          TTF_Text *text =
              TTF_CreateText(ttf_engine, ttf_font, mu_cmd->text.str, 0);
          if (!text) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                                     "Could not create text", nullptr);
            return 1;
          }
          TTF_DrawRendererText(text, static_cast<float>(mu_cmd->text.pos.x),
                               static_cast<float>(mu_cmd->text.pos.y));
          break;
        }
        case MU_COMMAND_RECT: {
          SDL_SetRenderDrawColor(sdlState.renderer, mu_cmd->rect.color.r,
                                 mu_cmd->rect.color.g, mu_cmd->rect.color.b,
                                 mu_cmd->rect.color.a);
          SDL_FRect rect{0};
          rect.x = static_cast<float>(mu_cmd->rect.rect.x);
          rect.y = static_cast<float>(mu_cmd->rect.rect.y);
          rect.w = static_cast<float>(mu_cmd->rect.rect.w);
          rect.h = static_cast<float>(mu_cmd->rect.rect.h);
          SDL_RenderFillRect(sdlState.renderer, &rect);
          break;
        }
        case MU_COMMAND_ICON: {
          break;
        }
        case MU_COMMAND_CLIP: {
          SDL_SetRenderClipRect(
              sdlState.renderer,
              reinterpret_cast<const SDL_Rect *>(&mu_cmd->clip.rect));
          break;
        }
      }
    }

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
