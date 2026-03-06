#include "TileMap.h"
#include <fstream>
#include <sstream>

bool TileMap::LoadFromFile(const char* filename, int width, int height)
{
	width = width;
	height = height;

	//assign은 벡터의 모든 요소를 특정 값으로 초기화하는 함수.
	tiles.assign(width * height, TileId::Empty);
	std::ifstream in(filename);
	if (!in.is_open())
	{
		return false;
	}
	std::string line;
	int y = 0;
	while (std::getline(in, line) && y < height)
	{
		std::istringstream ss(line);
		int x = 0;
		int tileInt;
		while (ss >> tileInt && x < width)
		{
			tiles[Index(x, y)] = static_cast<TileId>(tileInt);
			x++;
		}
		y++;
	}
	return true;
}

TileId TileMap::GetTile(int tx, int ty) const
{
	// 바깥은 벽 취급 (퐁 벽 역할).
	if (tx < 0 || tx >= width || ty < 0 || ty >= height)
		return TileId::SolidWall;
	return tiles[Index(tx, ty)];
}

TileProps TileMap::GetProps(TileId id) const
{
	TileProps p;
	switch (id)
	{
		case TileId::Empty:
			p.isSolid = false;
			p.speedMul = 1.0f;
			p.extraCost = 0;
			break;
		case TileId::SolidWall:
			p.isSolid = true;
			p.speedMul = 0.0f;
			p.extraCost = 100; // A*에서 사실상 못 지나가는 타일.
			break;
		case TileId::SoftWall:
			p.isSolid = true;
			p.speedMul = 0.5f;
			p.extraCost = 10; // A*에서 높은 비용.
			break;
		case TileId::SpeedZone:
			p.isSolid = false;
			p.speedMul = 1.5f;
			p.extraCost = 0;
			break;
		case TileId::SlowZone:
			p.isSolid = false;
			p.speedMul = 0.5f;
			p.extraCost = 0;
			break;
		default :
			break;
	}
	return p;
}

Int2 TileMap::WorldToTile(const Float2& p) const
{
	return { (int)(p.x / tileSize), (int)(p.y / tileSize)};
}
