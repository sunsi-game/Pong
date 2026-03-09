#include "TileMap.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

static inline void Trim(std::string& s)
{
	while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || std::isspace((unsigned char)s.back())))
	{
		s.pop_back();
	}

	size_t i = 0;
	while (i < s.size() && std::isspace((unsigned char)s[i]))
	{
		i++;
	}
	if (i > 0)
	{
		s.erase(0, i);
	}

}

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
			p.restitution = 1.0f;
			p.extraCost = 0;
			break;
		case TileId::SolidWall:
			p.isSolid = true;
			p.speedMul = 1.0f;
			p.restitution = 1.0f;
			p.extraCost = 100; // A*에서 사실상 못 지나가는 타일.
			break;
		case TileId::SoftWall:
			p.isSolid = true;
			p.speedMul = 0.5f;
			p.restitution = 1.0f;
			p.extraCost = 10; // A*에서 높은 비용.
			break;
		case TileId::SpeedZone:
			p.isSolid = false;
			p.speedMul = 1.5f;
			p.restitution = 1.0f;
			p.extraCost = 0;
			break;
		case TileId::SlowZone:
			p.isSolid = false;
			p.speedMul = 0.8f;
			p.restitution = 1.0f;
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

bool TileMap::LoadFromCSV(const char* path, int w, int h)
{
	width = w;
	height = h;
	tiles.assign(width * height, TileId::Empty);

	std::ifstream in(path);
	if (!in.is_open()) return false;

	std::string line;
	int y = 0;

	while (std::getline(in, line) && y < height)
	{
		Trim(line);

		// 빈 줄/주석 줄 스킵.
		if (line.empty() || line[0] == '#')
			continue;

		std::stringstream ss(line);

		for (int x = 0; x < width; ++x)
		{
			std::string cell;
			if (!std::getline(ss, cell, ',')) break;

			Trim(cell);
			if (cell.empty()) cell = "0";

			int v = std::stoi(cell);
			tiles[Index(x, y)] = static_cast<TileId>(v);
		}

		y++;
	}

	return (y == height); // 행 수 부족하면 false로 처리하고 싶을 때.
}

void TileMap::FillTestMap(int w, int h, int uiTopRows)
{
	width = w;
	height = h;
	tiles.assign(width * height, TileId::Empty);

	int playTop = uiTopRows;
	int playBottom = height - 1;

	// 외곽 벽
	for (int x = 0; x < width; ++x)
	{
		tiles[Index(x, playTop)] = TileId::SolidWall;
		tiles[Index(x, playBottom)] = TileId::SolidWall;
	}

	for (int y = playTop; y <= playBottom; ++y)
	{
		tiles[Index(0, y)] = TileId::SolidWall;
		tiles[Index(width - 1, y)] = TileId::SolidWall;
	}

	// 중앙 슬로우 존
	int midX = width / 2;
	for (int y = playTop + 1; y < playBottom; ++y)
	{
		for (int x = midX - 2; x <= midX + 2; ++x)
		{
			if (x > 0 && x < width - 1)
				tiles[Index(x, y)] = TileId::SlowZone;
		}
	}

	// 왼쪽 스피드 존
	for (int y = playTop + 2; y < playBottom - 1; ++y)
	{
		for (int x = 4; x <= 7; ++x)
		{
			if (x > 0 && x < width - 1)
				tiles[Index(x, y)] = TileId::SpeedZone;
		}
	}

	// 오른쪽 스피드 존
	for (int y = playTop + 2; y < playBottom - 1; ++y)
	{
		for (int x = width - 8; x <= width - 5; ++x)
		{
			if (x > 0 && x < width - 1)
				tiles[Index(x, y)] = TileId::SpeedZone;
		}
	}

	// 내부 테스트 벽
	int wallX = midX + 8;
	for (int y = playTop + 5; y < playTop + 10 && y < playBottom; ++y)
	{
		if (wallX > 0 && wallX < width - 1)
			tiles[Index(wallX, y)] = TileId::SolidWall;
	}
}
