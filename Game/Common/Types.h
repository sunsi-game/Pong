#pragma once
struct Int2
{
	int x;
	int y;

	bool operator==(const Int2& other) const
	{
		return x == other.x && y == other.y;
	}
};

struct Float2
{
	float x;
	float y;

	Float2() : x(0.0f), y(0.0f) {}
	Float2(float inX, float inY) : x(inX), y(inY) {}
};