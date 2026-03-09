#pragma once
#include "IPaddleAI.h"
#include <vector>

class AStarPositioningAI : public IPaddleAI
{
public:
    PaddleInput Update(
        float dt,
        const TileMap& map,
        const Float2& paddlePos,
        const Float2& ballPos,
        const Float2& ballVel) override;

	const std::vector<Int2>& GetDebugPath() const { return debugPath; }

    //void RequestPathRefresh() { refreshPath = true; }

private :
	std::vector<Int2> debugPath;/*
    bool refreshPath = true;*/
    float debugPathRefreshTimer = 0.0f;
    float debugPathRefreshInterval = 0.5f;
};