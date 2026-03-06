#pragma once
#include <memory>

#include "World/TileMap.h"
#include "Actor/Ball.h"
#include "Actor/Paddle.h"
#include "AI/IPaddleAI.h"

// 맵은 타일 데이터로 유지.
// 공은 연속좌표 (벡터).
// 충돌 후보는 공 주변 타일만 검사.
// AI는 (simple / A*) 선택.


namespace KhyPong
{
	class PongWorld
	{
	public :
		bool Init();
		void ResetRound();

		void Tick(float deltaTime);
		void Draw();

	private:
		void HandleInput(float deltaTime);
		void UpdateGameplay(float deltaTime);
		void HandleSecoreAndReset();

	private:
		TileMap map;
		Ball ball;
		Paddle left;
		Paddle right;

		int leftScore = 0;
		int rightScore = 0;
		int scoreToWin = 5;

		// AI (오른쪽 패들 예시).
		std::unique_ptr<IPaddleAI> rightAI;

		// 옵션.
		bool useAStarAI = true;
		bool multiBallMode = false; // 확장 모드 자리.

		// 월드 크기 (픽셀 기준).
		float worldW = 0.0f;
		float worldH = 0.0f;
	};
}

