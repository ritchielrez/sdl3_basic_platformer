#pragma once

#include "DeathScene.h"
#include "GameScene.h"
#include "ResourceManager.h"
#include "SDLState.h"
#include "StartScene.h"

enum class SceneType {
  start,
  game,
  death,
};

class SceneManager {
  const SDLState &sdlState;
  const ResourceManager &resourceManager;
  SceneType sceneType;
  StartScene startScene;
  GameScene gameScene;
  DeathScene deathScene;

 public:
  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sdlState(sdlState),
        resourceManager(resourceManager),
        sceneType(SceneType::game),
        startScene(),
        gameScene(sdlState, resourceManager),
        deathScene() {}

  void update(float dt) {
    switch (sceneType) {
      case SceneType::start:
        // startScene.update(dt);
        break;
      case SceneType::game:
        gameScene.update(dt);
        break;
      case SceneType::death:
        // deathScene.update(dt);
        break;
    }
  }
  void draw() {
    switch (sceneType) {
      case SceneType::start:
        // startScene.draw();
        break;
      case SceneType::game:
        gameScene.draw();
        break;
      case SceneType::death:
        // deathScene.draw();
        break;
    }
  }
};
