# Proposed Game Mechanics for SDL3 Platformer

This document outlines potential mechanics to be implemented within a one-month timeframe to enhance "game feel" and gameplay variety.

## 1. Movement Polish (Game Feel)
*   **Coyote Time**: Allow jumping for a brief window (0.1s) after leaving a platform.
*   **Jump Buffering**: Buffer a jump input if pressed shortly before landing.
*   **Variable Jump Height**: Control jump height by holding/releasing the jump button.

## 2. Mobility Enhancements
*   **Double Jump**: Allow one additional jump in mid-air.
*   **Dash**: A quick horizontal dash (e.g., 'Shift' key) with a cooldown.
*   **Wall Slide/Jump**: Slide down walls and jump off them to gain height.

## 3. Combat & Interaction
*   **Enemy Stomping**: Destroy enemies by landing on them while falling.
*   **Power-up Fruit**: Use existing fruit assets for speed boosts or temporary invincibility.
*   **Destructible Blocks**: Tiles that break after contact or standing on them.

## 4. Level Mechanics
*   **Moving Platforms**: Complete the dynamic tile collision logic.
*   **One-Way Platforms**: Platforms that can be jumped through from below but stood upon.

---
**Status**: Proposed
**Deadline**: ~1 Month
