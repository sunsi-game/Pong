#pragma once
#include "IPaddleAI.h"

class AStarPositioningAI : public IPaddleAI
{
public:
    PaddleInput Update(float dt,
        const TileMap& map,
        const Float2& paddlePos,
        const Float2& ballPos,
        const Float2& ballVel) override
    {
        // 1) 공이 내 라인에 도착할 y를 예측
        // 2) y 근처에서 좋은 타일(슬로우 피하고 스피드 존 활용) 목표 선택
        // 3) A*로 경로 만들고 다음 타일 방향으로 이동

        // 지금은 뼈대: 일단 Simple하게 따라가되,
        // 나중에 A* 완성하면 여기 로직 바꾸면 됨.
        PaddleInput out;
        float dy = ballPos.y - paddlePos.y;
        if (dy > 3.0f) out.moveY = +1.0f;
        else if (dy < -3.0f) out.moveY = -1.0f;
        else out.moveY = 0.0f;
        return out;
    }
};