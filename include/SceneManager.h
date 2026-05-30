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
      : sceneType(SceneType::start),
        startScene(sdlState),
        gameScene(sdlState, resourceManager),
        deathScene(sdlState) {}

  void update(float dt) {
    switch (sceneType) {
      case SceneType::start: {
        startScene.update(dt);
        if (startScene.shouldStartGame) {
          sceneType = SceneType::game;
          startScene.shouldStartGame = false;
        }
        break;
      }
      case SceneType::game: {
        const Player &player = gameScene.player;
        gameScene.update(dt);
        if (player.death && player.anims[player.currAnim].isTimeOut())
          sceneType = SceneType::death;
        break;
      }
      case SceneType::death: {
        deathScene.update(dt);
        if (deathScene.shouldRetry) {
          gameScene.reset();
          sceneType = SceneType::game;
          deathScene.shouldRetry = false;
        } else if (deathScene.shouldBeBackToStart) {
          gameScene.reset();
          sceneType = SceneType::start;
          deathScene.shouldBeBackToStart = false;
        }
        break;
      }
    }
  }

  void handleEvent(const SDL_Event &event) {
    switch (sceneType) {
      case SceneType::start:
        startScene.handleEvent(event);
        break;
      case SceneType::death:
        deathScene.handleEvent(event);
        break;
      default:
        break;
    }
  }

  void draw() {
    switch (sceneType) {
      case SceneType::start:
        startScene.draw();
        break;
      case SceneType::game:
        gameScene.draw();
        break;
      case SceneType::death:
        deathScene.draw();
        break;
    }
  }
};
