#pragma once

#include "DeathScene.h"
#include "GameScene.h"
#include "MainScene.h"
#include "ResourceManager.h"
#include "SDLState.h"

enum class SceneType { MAIN, GAME, DEATH };

class SceneManager {
  SceneType sceneType;
  union {
    MainScene mainScene;
    GameScene gameScene;
    DeathScene deathScene;
  };

 public:
  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sceneType(SceneType::GAME), gameScene(sdlState, resourceManager) {}
};
