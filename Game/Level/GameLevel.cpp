#include "GameLevel.h"
#include "Render/Renderer.h"
#include "Engine/Engine.h"
#include "Game/GameManager.h"
#include "World/PongWorld.h"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

GameLevel::GameLevel()
{
	world = new PongWorld();
}

GameLevel::~GameLevel()
{
	delete world;
	world = nullptr;
}

void GameLevel::BeginPlay()
{
	if (started) return;
	started = true;

	world->Init();
	world->ResetRound();
}

void GameLevel::Tick(float deltaTime)
{
	// 입력, 업데이트, 충돌, AI 한 번에 처리.
	world->Tick(deltaTime);
}

void GameLevel::Draw()
{
	world->Draw();
}
