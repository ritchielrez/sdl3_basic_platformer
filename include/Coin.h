#pragma once

#include "Entity.h"

// Type alias for collectible coins. Coins are plain entities — they don't
// need custom behavior since they're static objects that the player picks
// up on contact (handled in Player::collision). Using a type alias rather
// than a raw Entity makes the intent clear at call sites (e.g.
// std::vector<Coin> vs std::vector<Entity>).
using Coin = Entity;
