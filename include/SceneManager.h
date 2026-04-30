#pragma once

#include "DeathScene.h"
#include "GameScene.h"
#include "MainScene.h"

enum class SceneType { MAIN, GAME, DEATH };

class SceneManager {
  SceneType sceneType;
  union {
    MainScene main;
    GameScene game;
    DeathScene death;
  } scene;

  SceneManager() : sceneType(SceneType::GAME) { scene.game = GameScene(); }
};
