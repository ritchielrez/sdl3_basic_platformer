#pragma once

#include "Entity.h"

// Type alias for immovable terrain tiles (ground, walls, platforms). These
// are the collision geometry of the level — the player and enemies collide
// with them but they never move. Like Coin, no custom logic is needed beyond
// what Entity provides.
using StaticTile = Entity;
