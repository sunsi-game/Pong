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
};