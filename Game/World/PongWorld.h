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
		void HandleSecoreAndReset(float deltaTime);
		void DrawDebug();
		void DrawZonesDebug();
		void ApplyZonesToBall(float deltaTime);


	private:

		struct ZoneRect
		{
			int x0, y0, x1, y1; // 포함 범위
			float speedMul;     // 0.8 슬로우 / 1.2 스피드
			char debugChar;     // 표시용
		};

		std::vector<ZoneRect> zones;

		TileMap map;
		Ball ball;
		Paddle left;
		Paddle right;

		int leftScore = 0;
		int rightScore = 0;
		int scoreToWin = 5;

		// 누가 서브를 할지.
		int serveDir = 1; // 1이면 오른쪽으로, -1이면 왼쪽으로 서브.

		// AI (오른쪽 패들 예시).
		std::unique_ptr<IPaddleAI> rightAI;

		// 옵션.
		bool useAStarAI = true;
		bool multiBallMode = false; // 확장 모드 자리.

		// 월드 크기 (픽셀 기준).
		float worldW = 0.0f;
		float worldH = 0.0f;

		float serveTimer = 0.0f; // 서브 타이머 (공이 잠시 멈춰있는 시간).
		bool waitingServe = false; // 서브 대기 상태 여부.

		bool matchEnded = false;
		float endTimer = 0.0f;
		bool leftWon = false; // true면 왼쪽 승, false면 오른쪽 승

		bool dbgGrid = false;      // F2: 타일 그리드
		bool dbgBallBox = false;   // F4: 공 AABB/검사 영역
		bool dbgZones = false;     // F3: 존 표시(스피드/슬로우)
	};
}

