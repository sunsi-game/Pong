#pragma once
#include <cstdint>

enum class TileId : uint8_t
{
	Empty = 0,
	SolidWall = 1,
	SoftWall = 2,
	SpeedZone = 3,
	SlowZone = 4,
};

struct TileProps
{
	bool isSolid = false;
	float speedMul = 1.0f;
	float restitution = 1.0f;
	int extraCost = 0; // A* 알고리즘에서 추가 비용.
};