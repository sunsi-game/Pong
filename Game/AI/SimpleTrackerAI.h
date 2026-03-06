#pragma once
#include "IPaddleAI.h"
#include <cmath>

class SimpleTrackerAI : public IPaddleAI
{
public:
    PaddleInput Update(float, const TileMap&, const Float2& paddlePos,
        const Float2& ballPos, const Float2&) override
    {
        PaddleInput out;
        float dy = ballPos.y - paddlePos.y;
        if (dy > 3.0f) out.moveY = +1.0f;
        else if (dy < -3.0f) out.moveY = -1.0f;
        else out.moveY = 0.0f;
        return out;
    }
};