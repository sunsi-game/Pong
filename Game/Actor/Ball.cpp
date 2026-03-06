#include "Ball.h"
#include <algorithm>
#include <cmath>

static float Clamp(float v, float min, float max)
{
	return std::max(min, std::min(max, v));
}

void Ball::Reset(const Float2& p, const Float2& dir, float speed)
{
	pos = p;
	vel = { dir.x * speed, dir.y * speed };
}

void Ball::Tick(float deltaTime, const TileMap& map)
{
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
    // 패들은 AABB, 공은 원(여기서는 원을 AABB로 대충)
    float pMinX = p.GetPos().x - p.GetHalfW();
    float pMaxX = p.GetPos().x + p.GetHalfW();
    float pMinY = p.GetPos().y - p.GetHalfH();
    float pMaxY = p.GetPos().y + p.GetHalfH();

    float bMinX = pos.x - radius;
    float bMaxX = pos.x + radius;
    float bMinY = pos.y - radius;
    float bMaxY = pos.y + radius;

    if (!AABBOverlap(bMinX, bMinY, bMaxX, bMaxY, pMinX, pMinY, pMaxX, pMaxY))
        return;

    // x 반전 + y 방향 살짝 꺾기(맞은 위치에 따라)
    vel.x = -vel.x;

    float offset = (pos.y - p.GetPos().y) / p.GetHalfH(); // -1~+1
    vel.y += offset * 40.0f;
}