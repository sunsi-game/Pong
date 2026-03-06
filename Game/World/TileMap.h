#pragma once
#include <vector>
#include "Common/Types.h"
#include "TileTpyes.h"

class TileMap
{
public :
	bool LoadFromFile(const char* filename, int width, int height);

	inline void SetTileSize(int v) { tileSize = v; }
	inline int GetTileSize() const { return tileSize; }

	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

	TileId GetTile(int tx, int ty) const;
	TileProps GetProps(TileId id) const;

	Int2 WorldToTile(const Float2& p) const;

private :
	int width = 0;
	int height = 0;
	int tileSize = 16;
	std::vector<TileId> tiles;

private :
	inline int Index(int x, int y) const { return y * width + x; }
};

