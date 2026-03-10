#pragma once
#include "Common/Types.h"
#include "World/TileMap.h"
#include "Paddle.h"
#include "World/Obstacle.h"
#include "World/PowerUp.h"

class Ball
{
public :
	void Reset(const Float2& p, const Float2& dir, float speed);
	void Tick(float deltaTime, const TileMap& map);
	void Tick(float deltaTime, float worldW, float worldH);

	inline const Float2& GetPos() const { return pos; }
	inline const Float2& GetVel() const { return vel; }

	void ResolvePaddleCollision(const Paddle& paddle);

	inline void SetVel(const Float2& newVel) { vel = newVel; }
	inline int GetGimmickLoopStreak() const { return gimmickLoopStreak;  }
	inline float GetRadius() const { return radius; }

	inline void SetPos(const Float2& p) { pos = p; }

	void ReflectX();
	void ReflectY();
	void MultiplySpeed(float scale);

private :
	Float2 prevPos{ 0,0 };
	Float2 pos{ 0,0 };
	Float2 vel{ 0,0 };
	float radius = 2.0f;
	Int2 prevTile{ -999, -999 };
	int wallBounceStreak = 0; // 연속 벽 반사 횟수.
	int gimmickLoopStreak = 0;


private : 
	void ResolveTileCollision(const TileMap& map, const Float2& oldPos);


};

