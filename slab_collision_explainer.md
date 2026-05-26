# Slab Collision: How It Works & How to Use It

## What Is "Slab" Collision?

The **slab method** (also called the **slab test** or **slab intersection**) is a technique for computing the **exact time of collision** between two axis-aligned bounding boxes (AABBs) along a ray or velocity vector. It originates from ray-AABB intersection tests in ray-tracing, but is extremely useful for platformer physics.

The core idea: instead of moving the player first and then asking *"are we overlapping?"*, you ask **"at what fraction of this frame's movement will we first touch a tile, and on which axis?"**

---

## Your Current Approach vs. Slab

### Current approach: Overlap Resolution (Intersection Rect)

Your [Player::collision](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L211-L321) does this:

1. **Move the player** by the full `vel * dt` ([line 157](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L157)).
2. **Check for overlaps** with every tile using `SDL_GetRectIntersectionFloat`.
3. **Resolve** based on the intersection rect dimensions:
   - If `intersectionRect.w > intersectionRect.h` → resolve vertically (push out on Y).
   - Otherwise → resolve horizontally (push out on X).

```
Frame start:          After move:           After resolution:
  ┌──┐                  ┌──┐                  ┌──┐
  │P │  ──vel──→     ▓▓▓│P │               │P │
  └──┘               ▓▓▓└──┘               └──┘▓▓▓
                     ▓▓▓▓▓▓▓               ▓▓▓▓▓▓▓
                     (overlap)             (pushed out)
```

### Known bugs with this approach

> [!WARNING]
> **Corner Clipping / Wrong-Axis Resolution**: When the player hits a corner (e.g., running into a wall while falling), the intersection rect can be almost square. A tiny floating-point difference determines whether you get pushed out on X or Y. This causes:
> - **Snapping to wall tops** when you should slide down them.
> - **Catching on tile seams** when running over adjacent ground tiles.
> - **Tunneling** at high velocities when `vel * dt > tile_size`.

> [!WARNING]
> **Order-Dependent Resolution**: You iterate tiles in array order. Resolving collision with tile A can push you into tile B, creating jitter between frames. Your code recalculates `playerCollider` after each tile ([line 252](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L252)), which helps, but doesn't eliminate the ordering problem.

---

## How the Slab Method Works

### The Core Intuition

Think of each axis independently. On the X-axis, the player's collider sweeps from its start X to its end X during the frame. A tile also occupies a range on the X-axis. We can compute the **time interval** during which the player overlaps the tile on just the X-axis. We do the same for the Y-axis. If those two time intervals overlap, there is a collision, and the **start of the combined interval** tells us *when* the collision happens.

### Step-by-Step Algorithm

Given:
- Player AABB: `(px, py, pw, ph)` — position and size at the start of the frame
- Player velocity this frame: `(vx, vy)` — i.e., `vel * dt`
- Tile AABB: `(tx, ty, tw, th)`

**Step 1: Compute entry and exit distances on each axis**

```
// How far the player needs to travel to START overlapping the tile
if vx > 0:
    xEntryDist = tx - (px + pw)          // left edge of tile minus right edge of player
    xExitDist  = (tx + tw) - px          // right edge of tile minus left edge of player
else:
    xEntryDist = (tx + tw) - px
    xExitDist  = tx - (px + pw)

// Same for Y axis
if vy > 0:
    yEntryDist = ty - (py + ph)
    yExitDist  = (ty + th) - py
else:
    yEntryDist = (ty + th) - py
    yExitDist  = ty - (py + ph)
```

**Step 2: Convert distances to times (0.0 = start of frame, 1.0 = end of frame)**

```
if vx == 0:
    xEntry = -infinity    // never enters on X (already overlapping or never will)
    xExit  = +infinity
else:
    xEntry = xEntryDist / vx
    xExit  = xExitDist / vx

// Same for Y
if vy == 0:
    yEntry = -infinity
    yExit  = +infinity
else:
    yEntry = yEntryDist / vy
    yExit  = yExitDist / vy
```

**Step 3: Find the overall entry and exit times**

```
entryTime = max(xEntry, yEntry)   // both axes must overlap simultaneously
exitTime  = min(xExit, yExit)     // collision ends when either axis stops overlapping
```

**Step 4: Determine if a collision actually occurs this frame**

```
if entryTime > exitTime:        return NO_COLLISION   // intervals don't overlap
if xEntry < 0 AND yEntry < 0:   return NO_COLLISION   // collision is in the past
if xEntry > 1 OR yEntry > 1:    return NO_COLLISION   // collision is beyond this frame
```

If none of those conditions are true, **`entryTime`** is the fraction of the frame at which the collision occurs (between 0.0 and 1.0).

**Step 5: Determine the collision normal (which face was hit)**

```
if xEntry > yEntry:
    // X-axis was the last to enter → collision is on a vertical face
    normal = (vx > 0) ? (-1, 0) : (1, 0)
else:
    // Y-axis was the last to enter → collision is on a horizontal face
    normal = (vy > 0) ? (0, -1) : (0, 1)
```

This is the critical advantage: **the axis that entered last is definitively the collision axis**. No ambiguity, no guessing based on intersection rect shape.

### Visual Example

```
Frame start:              Slab analysis:

  ┌──┐                    X-axis overlap: t=0.3 to t=0.9
  │P │ ──(vx,vy)──→      Y-axis overlap: t=0.5 to t=1.2
  └──┘           ▓▓▓
                 ▓▓▓      entryTime = max(0.3, 0.5) = 0.5
                 ▓▓▓      exitTime  = min(0.9, 1.2) = 0.9

                           0.5 < 0.9 → collision at t=0.5
                           yEntry(0.5) > xEntry(0.3) → hit TOP face
                           normal = (0, -1)
```

---

## How Resolution Works With Slab

Once you have `entryTime` and `normal`:

1. **Move the player only up to the collision point**:
   ```
   pos.x += vx * entryTime
   pos.y += vy * entryTime
   ```

2. **Zero out velocity on the collision axis**:
   ```
   if normal.x != 0:  vel.x = 0   // hit a wall
   if normal.y != 0:  vel.y = 0   // hit floor or ceiling
   ```

3. **Use remaining time to slide** along the non-colliding axis:
   ```
   remainingTime = 1.0 - entryTime
   pos.x += vel.x * dt * remainingTime   // (if normal was on Y, vel.x is still intact)
   pos.y += vel.y * dt * remainingTime
   ```

4. **Repeat** the slab test for the sliding movement (handles sliding into a second tile, e.g., corners). Usually 2-3 iterations is sufficient.

---

## How To Apply This to Your Codebase

### What changes

| Component | Current | With Slab |
|---|---|---|
| Movement | Move first, then resolve overlaps | Sweep-test before moving |
| Collision axis | Guessed from intersection rect shape | Definitively determined by entry times |
| Resolution | Push player out of overlap | Stop player at contact point, slide remainder |
| Ground detection | Post-resolution position check | Normal vector `(0, -1)` = landed on ground |
| Multi-tile | Order-dependent, tile-by-tile | Sort by `entryTime`, resolve nearest first |

### Files that would change

#### [Player.cpp](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp)

The main change. The `collision()` method ([L211-L321](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L211-L321)) would be rewritten to:
1. **Not** move `pos` by `velFrame` before collision — instead pass `velFrame` into collision.
2. Sweep-test against all static tiles, find the one with the **smallest `entryTime`**.
3. Resolve (move to contact, slide), then repeat for remaining velocity.
4. Ground detection becomes: did any collision produce `normal.y == -1`?

The `update()` method ([L18-L209](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L18-L209)) would change at [line 150-159](file:///home/ritchiel/repos/sdl3_basic_platformer/src/Player.cpp#L150-L159) — instead of `pos += velFrame` then `collision()`, you'd call `collision(velFrame)` which handles movement internally.

#### [DynTile.h](file:///home/ritchiel/repos/sdl3_basic_platformer/include/DynTile.h) and [Slime.h](file:///home/ritchiel/repos/sdl3_basic_platformer/include/Slime.h)

These have their own `collision()` methods ([DynTile L24-L55](file:///home/ritchiel/repos/sdl3_basic_platformer/include/DynTile.h#L24-L55), [Slime L26-L57](file:///home/ritchiel/repos/sdl3_basic_platformer/include/Slime.h#L26-L57)) with the same intersection-rect pattern. They would benefit from the same refactor, but since they only move horizontally, the current approach may be "good enough" for them.

### Pseudocode for the refactored collision loop

```cpp
void Player::collision(glm::vec2 velFrame, /*...*/) {
    constexpr int MAX_ITERATIONS = 3;

    for (int iter = 0; iter < MAX_ITERATIONS && glm::length(velFrame) > 0.0001f; iter++) {
        float nearestTime = 1.0f;  // default: move the full frame
        glm::vec2 nearestNormal{0, 0};

        for (auto& tile : staticTiles) {
            float entryTime;
            glm::vec2 normal;
            if (sweptAABB(playerCollider, velFrame, tileRect, entryTime, normal)) {
                if (entryTime < nearestTime) {
                    nearestTime = entryTime;
                    nearestNormal = normal;
                }
            }
        }

        // Move to the nearest collision point (or full distance if no collision)
        pos += velFrame * nearestTime;

        if (nearestNormal.y == -1.0f) foundGround = true;

        // Slide: remove the component of velocity along the collision normal
        if (nearestNormal.x != 0) { velFrame.x = 0; vel.x = 0; }
        if (nearestNormal.y != 0) { velFrame.y = 0; vel.y = 0; }

        // Scale remaining velocity by remaining time
        velFrame *= (1.0f - nearestTime);
    }
}
```

---

## Edge Cases to Be Aware Of

| Edge Case | How slab handles it |
|---|---|
| **Zero velocity on one axis** | Entry = −∞, Exit = +∞ on that axis. The test degrades to a 1D slab on the other axis. |
| **Already overlapping at frame start** | Both entry times are negative → no collision detected. You need a separate static overlap resolver as a fallback (depenetration). |
| **Tile seams** (running over adjacent ground tiles) | Because the collision normal is unambiguous, the player won't "catch" on seams between tiles at the same Y level. |
| **Diagonal into a corner** | The axis with the later entry time wins. The player slides along the wall cleanly. |
| **Very high velocity** | Works perfectly — it's a parametric test, not a discrete overlap. No tunneling. |

> [!IMPORTANT]
> **Static overlap fallback**: The slab method is a *swept* test — it detects collisions during movement. If the player starts a frame already inside a tile (e.g., spawned inside geometry, or pushed by a moving platform), the swept test won't catch it. You should keep a simple depenetration pass (similar to your current code) as a safety net for this case.

---

## Summary

The slab method replaces your current "move → overlap? → push out" loop with a "how far can I move before hitting something?" sweep test. The key wins are:

1. **No axis ambiguity** — the collision normal is mathematically determined
2. **No tunneling** — works at any velocity
3. **Clean sliding** — remaining velocity naturally slides along surfaces
4. **No tile-seam catching** — adjacent ground tiles don't create false wall collisions
5. **Better ground detection** — `normal.y == -1` is ground, period

The main cost is slightly more code and needing an iteration loop (typically 2-3 passes) for sliding into secondary collisions.
