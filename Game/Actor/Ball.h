#pragma once
#include "Common/Types.h"
#include "World/TileMap.h"
#include "Paddle.h"

class Ball
{
public :
	void Reset(const Float2& p, const Float2& dir, float speed);
	void Tick(float deltaTime, const TileMap& map);

	inline const Float2& GetPos() const { return pos; }
	inline const Float2& GetVel() const { return vel; }

	void ResolvePaddleCollision(const Paddle& paddle);

private :
	Float2 pos{ 0,0 };
	Float2 vel{ 0,0 };
	float radius = 3.0f;

private : 
	void ResolveTileCollision(const TileMap& map, const Float2& oldPos);
};

