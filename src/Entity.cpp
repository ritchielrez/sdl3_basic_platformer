#include "Entity.h"

#include "Game.h"
#include "Log.h"

void Entity::draw(const SDLState &sdlState) {
  // NOTE: An without an actual animation has a `currAnim` of -1.
  // However it still has an initialized `anims` object, so we can access
  // texture coordinates for that
  Frames currFrame = anims[currAnim != -1 ? currAnim : 0];
  SDL_FRect src{.x = currFrame.getTexCoord().x,
                .y = currFrame.getTexCoord().y,
                .w = currFrame.getFrameWidth(),
                .h = currFrame.getFrameHeight()};
  SDL_FRect dst{.x = pos.x, .y = pos.y, .w = w, .h = h};

  SDL_FlipMode flipMode = dir == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RenderTextureRotated(sdlState.renderer, tex, &src, &dst, 0, nullptr,
                           flipMode);

  if (Game::debug) {
    SDL_FRect colliderRect{0};
    colliderRect.x = pos.x + collider.x;
    colliderRect.y = pos.y + collider.y;
    colliderRect.w = collider.w;
    colliderRect.h = collider.h;
    SDL_SetRenderDrawColor(sdlState.renderer, 255, 0, 0, 128);
    SDL_RenderFillRect(sdlState.renderer, &colliderRect);
  }
}
