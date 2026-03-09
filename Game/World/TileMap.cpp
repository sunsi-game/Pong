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
	// 위/아래 바깥은 벽
	if (ty < 0 || ty >= height)
		return TileId::SolidWall;

	// 좌/우 바깥은 비움(득점 처리용)
	if (tx < 0 || tx >= width)
		return TileId::Empty;

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
			p.isSolid = false;
			p.speedMul = 0.5f;
			p.restitution = 1.0f;
			p.extraCost = 10; // A*에서 높은 비용.
			break;
		case TileId::StickyZone:
			p.isSolid = false;
			p.speedMul = 0.94f;
			p.restitution = 1.0f;
			p.extraCost = 2;
			break;

		case TileId::WindRight:
			p.isSolid = false;
			p.speedMul = 1.0f;
			p.force = { 6.0f, 0.0f };
			p.extraCost = 1;
			break;

		case TileId::WindLeft:
			p.isSolid = false;
			p.speedMul = 1.0f;
			p.force = { -6.0f, 0.0f };
			p.extraCost = 1;
			break;

		case TileId::Bumper:
			p.isSolid = false;
			p.speedMul = 1.0f;
			p.restitution = 1.25f;
			p.extraCost = 20;
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

	const int playTop = uiTopRows;
	const int playBottom = height - 1;
	const int midX = width / 2;
	const int midY = (playTop + playBottom) / 2;

	auto SafeSet = [&](int x, int y, TileId id)
		{
			if (x >= 0 && x < width && y >= 0 && y < height)
				tiles[Index(x, y)] = id;
		};

	// =========================
	// 1) 위 / 아래 벽
	// =========================
	for (int x = 0; x < width; ++x)
	{
		SafeSet(x, playTop, TileId::SolidWall);
		SafeSet(x, playBottom, TileId::SolidWall);
	}

	// =========================
	// 2) 중앙 StickyZone (아주 작게)
	// =========================
	for (int y = midY - 1; y <= midY + 1; ++y)
		for (int x = midX; x <= midX + 1; ++x)
			SafeSet(x, y, TileId::StickyZone);

	// =========================
	// 3) 좌상단 WindRight 포켓
	// =========================
	for (int y = playTop + 3; y <= playTop + 5; ++y)
		for (int x = 6; x <= 8; ++x)
			SafeSet(x, y, TileId::WindRight);

	// =========================
	// 4) 우하단 WindLeft 포켓
	// =========================
	for (int y = playBottom - 5; y <= playBottom - 3; ++y)
		for (int x = width - 9; x <= width - 7; ++x)
			SafeSet(x, y, TileId::WindLeft);

	// =========================
	// 5) 우상단 짧은 세로 벽
	// =========================
	int wall1X = width - 12;
	for (int y = playTop + 4; y <= playTop + 6; ++y)
		SafeSet(wall1X, y, TileId::SolidWall);

	// =========================
	// 6) 좌하단 짧은 세로 벽
	// =========================
	int wall2X = 11;
	for (int y = playBottom - 6; y <= playBottom - 4; ++y)
		SafeSet(wall2X, y, TileId::SolidWall);

	// =========================
	// 7) 좌하단 Bumper
	// =========================
	SafeSet(midX - 10, midY + 4, TileId::Bumper);

	// =========================
	// 8) 우상단 Bumper
	// =========================
	SafeSet(midX + 8, midY - 4, TileId::Bumper);

	// =========================
	// 9) 우측 중앙 근처 SoftWall
	// =========================
	SafeSet(midX + 9, midY + 1, TileId::SoftWall);
	SafeSet(midX + 9, midY + 2, TileId::SoftWall);
}
