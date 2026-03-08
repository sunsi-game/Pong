#include "Ball.h"
#include <algorithm>
#include <cmath>

static float Clamp(float v, float a, float b)
{
	return (v < a) ? a : (v > b) ? b : v;
}

static float Sqrt(float v)
{
    return std::sqrt(v);
}

void Ball::Reset(const Float2& p, const Float2& dir, float speed)
{
	pos = p;
	vel = { dir.x * speed, dir.y * speed };
}

void Ball::Tick(float deltaTime, const TileMap& map)
{
    // 맵이 아직 로드/초기화 안 된 상태면 타일 충돌 스킵
    if (map.GetWidth() <= 0 || map.GetHeight() <= 0)
    {
        // 임시로 월드 반사(혹은 그냥 이동만) - Step A처럼 처리해도 됨
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;
        return;
    }

	Float2 old = pos;

	pos.x += vel.x * deltaTime;
	pos.y += vel.y * deltaTime;

	ResolveTileCollision(map, old);

	// 존 적용 (현재 타일의 speedMul).
	auto t = map.WorldToTile(pos);
	auto props = map.GetProps(map.GetTile(t.x, t.y));
	if (!props.isSolid && props.speedMul != 1.0f)
	{
		vel.x *= props.speedMul;
		vel.y *= props.speedMul;
	}

}

void Ball::Tick(float deltaTime, float worldW, float worldH)
{
    prevPos = pos;

	pos.x += (vel.x * deltaTime);
	pos.y += (vel.y * deltaTime);

    // 위, 아래 벽 반사.
	const float top = 0.0f + radius;
	const float bottom = (worldH - 1.0f) - radius;

    if (pos.y < top)
    {
        pos.y = top;
        vel.y = -vel.y;
    }
    else if (pos.y > bottom)
    {
        pos.y = bottom;
        vel.y = -vel.y;
	}
}

static bool AABBOverlap(float aMinX, float aMinY, float aMaxX, float aMaxY,
    float bMinX, float bMinY, float bMaxX, float bMaxY)
{
    return !(aMaxX < bMinX || aMinX > bMaxX || aMaxY < bMinY || aMinY > bMaxY);
}

void Ball::ResolveTileCollision(const TileMap& map, const Float2& oldPos)
{
    Float2 minP{ pos.x - radius, pos.y - radius };
    Float2 maxP{ pos.x + radius, pos.y + radius };

    Int2 tmin = map.WorldToTile(minP);
    Int2 tmax = map.WorldToTile(maxP);

    const float ts = (float)map.GetTileSize();

    for (int ty = tmin.y; ty <= tmax.y; ++ty)
        for (int tx = tmin.x; tx <= tmax.x; ++tx)
        {
            TileId id = map.GetTile(tx, ty);
            TileProps props = map.GetProps(id);
            if (!props.isSolid) continue;

            float bMinX = tx * ts;
            float bMinY = ty * ts;
            float bMaxX = bMinX + ts;
            float bMaxY = bMinY + ts;

            float aMinX = pos.x - radius;
            float aMinY = pos.y - radius;
            float aMaxX = pos.x + radius;
            float aMaxY = pos.y + radius;

            if (!AABBOverlap(aMinX, aMinY, aMaxX, aMaxY, bMinX, bMinY, bMaxX, bMaxY))
                continue;

            // 간단 처리: 이전 위치로 되돌리고 속도 반전(축 판단은 oldPos 기반)
            // (더 고급으로 하고 싶으면 swept/circle-aabb로 업그레이드하면 됨)
            pos = oldPos;

            // 어느 축 충돌인지 대충 판정
            // x 이동이 컸으면 x 반전, y 이동이 컸으면 y 반전
            float dx = std::abs((oldPos.x + vel.x) - oldPos.x);
            float dy = std::abs((oldPos.y + vel.y) - oldPos.y);

            if (dx >= dy) vel.x = -vel.x * props.restitution;
            else          vel.y = -vel.y * props.restitution;

            return;
        }
}

void Ball::ResolvePaddleCollision(const Paddle& p)
{
    float pMinX = p.GetPos().x - p.GetHalfW();
    float pMaxX = p.GetPos().x + p.GetHalfW();
    float pMinY = p.GetPos().y - p.GetHalfH();
    float pMaxY = p.GetPos().y + p.GetHalfH();

    // y 범위 먼저 체크(공 중심 기준)
    if (pos.y < pMinY - radius || pos.y > pMaxY + radius)
        return;

    // 왼쪽 패들 쪽으로 가는 중: 오른쪽 면(pMaxX)을 "통과"했는지
    if (vel.x < 0.0f)
    {
        bool wasRight = (prevPos.x - radius) > pMaxX;
        bool nowLeftOrInside = (pos.x - radius) <= pMaxX;

        if (wasRight && nowLeftOrInside)
        {
            // 충돌 처리
            pos.x = pMaxX + radius;

            float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            float offset = (pos.y - p.GetPos().y) / p.GetHalfH();
            offset = Clamp(offset, -1.0f, 1.0f);

            float angle = offset * (60.0f * 3.141592f / 180.0f);
            vel.x = +std::cos(angle) * speed;
            vel.y = std::sin(angle) * speed;
        }
    }
    // 오른쪽 패들 쪽으로 가는 중: 왼쪽 면(pMinX)을 통과했는지
    else if (vel.x > 0.0f)
    {
        bool wasLeft = (prevPos.x + radius) < pMinX;
        bool nowRightOrInside = (pos.x + radius) >= pMinX;

        if (wasLeft && nowRightOrInside)
        {
            pos.x = pMinX - radius;

            float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            float offset = (pos.y - p.GetPos().y) / p.GetHalfH();
            offset = Clamp(offset, -1.0f, 1.0f);

            float angle = offset * (60.0f * 3.141592f / 180.0f);
            vel.x = -std::cos(angle) * speed;
            vel.y = std::sin(angle) * speed;
        }
    }

}