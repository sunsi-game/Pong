#pragma once
#include <memory>
#include <vector>

#include "World/TileMap.h"
#include "Actor/Ball.h"
#include "Actor/Paddle.h"
#include "AI/IPaddleAI.h"
#include "ISpatialIndex.h"

class Obstacle;
class PowerUp;

namespace KhyPong
{
	class PongWorld
	{
	public:
		~PongWorld();
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
		void ApplyZonesToBall(Ball& ball, float deltaTime);

		// Step D
		void UpdateRoundTimer(float deltaTime);
		void UpdateBalls(float deltaTime);
		void UpdateBall(Ball& ball, float deltaTime);
		void RebuildSpatialIndex();
		void ToggleSpatialIndexMode();

		void SpawnBall(const Float2& pos, const Float2& dir, float speed);
		void SpawnTimedMultiBall();
		bool ShouldSpawnTimedMultiBall() const;

		bool IntersectsBallAABB(const Ball& ball, float left, float top, float right, float bottom) const;
		void ResolveBallVsObstacle(Ball& ball, const Obstacle& obstacle);
		bool CheckBallVsPowerUp(const Ball& ball, const PowerUp& powerUp) const;
		void ApplyPowerUp(Ball& ball, const PowerUp& powerUp);

		void BuildStepDObjects();

	private:
		struct ZoneRect
		{
			int x0, y0, x1, y1;
			float speedMul;
			char debugChar;
		};

		enum class SpatialIndexMode
		{
			None,
			Quadtree
		};

		std::vector<ZoneRect> zones;

		TileMap map;

		// 기존 단일 공은 유지하고, 실제 플레이는 balls로 확장.
		Ball ball;
		std::vector<Ball> balls;

		Paddle left;
		Paddle right;

		int leftScore = 0;
		int rightScore = 0;
		int scoreToWin = 5;

		int serveDir = 1;

		std::unique_ptr<IPaddleAI> rightAI;

		bool useAStarAI = true;
		bool multiBallMode = false;

		float worldW = 0.0f;
		float worldH = 0.0f;

		float serveTimer = 0.0f;
		bool waitingServe = false;

		bool matchEnded = false;
		float endTimer = 0.0f;
		bool leftWon = false;

		bool dbgGrid = false;
		bool dbgBallBox = false;
		bool dbgZones = false;
		bool dbgPath = false;

		// Step D
		SpatialIndexMode spatialMode = SpatialIndexMode::None;
		std::unique_ptr<ISpatialIndex> spatialIndex;

		std::vector<std::unique_ptr<Obstacle>> obstacles;
		std::vector<std::unique_ptr<PowerUp>> powerUps;

		float roundTimeLimit = 35.0f;
		float roundTimeRemaining = 35.0f;
		bool roundEnded = false;
		int timedMultiBallStage = 0; // 20 / 15 / 10 / 5초
		int maxBalls = 8;

		int bruteForceChecks = 0;
		int spatialCandidateChecks = 0;
		int actualCollisionCount = 0;
		int obstacleCandidateCount = 0;
		int powerUpCandidateCount = 0;
	};
}