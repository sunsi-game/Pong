#pragma once
#include <cstdint>

enum class TileId : uint8_t
{
	Empty = 0,
	SolidWall = 1,
	SoftWall = 2,
	StickyZone = 3,
	WindRight = 4,
	WindLeft = 5,
	Bumper = 6
};

struct TileProps
{
	bool isSolid = false;
	float speedMul = 1.0f;
	float restitution = 1.0f;
	int extraCost = 0; // A* 알고리즘에서 추가 비용.

	Float2 force{0.0f, 0.0f};
};