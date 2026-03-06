#pragma once
#include "Common/Types.h"
#include "World/TileMap.h"

struct PaddleInput
{
	float moveY = 0.0f;
};

class IPaddleAI
{
public :
	virtual ~IPaddleAI() = default;
	virtual PaddleInput Update(
		float deltatime,
		const TileMap& map,
		const Float2& paddlePos,
		const Float2& ballPos,
		const Float2& ballVel
	) = 0;
};

