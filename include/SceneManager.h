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
  SceneType sceneType;

 public:
  StartScene startScene;
  GameScene gameScene;
  DeathScene deathScene;

  SceneManager(const SDLState &sdlState, const ResourceManager &resourceManager)
      : sceneType(SceneType::game),
        startScene(),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState) {}

  void update(float dt) {
    switch (sceneType) {
      case SceneType::start:
        // startScene.update(dt);
        break;
      case SceneType::game:
        gameScene.update(dt);
        if (gameScene.player.death) sceneType = SceneType::death;
        break;
      case SceneType::death:
        deathScene.update(dt);
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
