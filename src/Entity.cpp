#include "Entity.h"

#include <glm/glm.hpp>

#include "Game.h"
#include "SDL3/SDL_rect.h"

void Entity::draw(const SDLState &sdlState, const SDL_FRect &cam) {
  // NOTE: An without an actual animation has a `currAnim` of -1.
  // However it still has an initialized `anims` object, so we can access
  // texture coordinates for that
  Frames currFrame = anims[currAnim != -1 ? currAnim : 0];
  SDL_FRect src{.x = glm::round(currFrame.getTexCoord().x),
                .y = glm::round(currFrame.getTexCoord().y),
                .w = static_cast<float>(currFrame.getFrameWidth()),
                .h = static_cast<float>(currFrame.getFrameHeight())};
  SDL_FRect dst{.x = pos.x - cam.x, .y = pos.y - cam.y, .w = w, .h = h};

  SDL_FlipMode flipMode = dir == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RenderTextureRotated(sdlState.renderer, tex, &src, &dst, 0, nullptr,
                           flipMode);

  if (Game::debug) {
    SDL_FRect colliderRect{0, 0, 0, 0};
    colliderRect.x = pos.x + collider.x - cam.x;
    colliderRect.y = pos.y + collider.y - cam.y;
    colliderRect.w = collider.w;
    colliderRect.h = collider.h;
    SDL_SetRenderDrawColor(sdlState.renderer, 255, 0, 0, 128);
    SDL_RenderFillRect(sdlState.renderer, &colliderRect);
  }
}
