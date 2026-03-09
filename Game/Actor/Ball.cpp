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

static float Length(const Float2& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

static void ClampBallSpeed(Float2& v, float minSpeed, float maxSpeed)
{
    float len = Length(v);
    if (len < 0.0001f) return;

    if (len < minSpeed)
    {
        float s = minSpeed / len;
        v.x *= s;
        v.y *= s;
    }
    else if (len > maxSpeed)
    {
        float s = maxSpeed / len;
        v.x *= s;
        v.y *= s;
    }
}

static void EnsureMinHorizontalSpeed(Float2& v, float minXSpeed)
{
    if (std::abs(v.x) >= minXSpeed)
        return;


    // 현재 x 방향 유지
    float signX = (v.x < 0.0f) ? -1.0f : 1.0f;
    if (std::abs(v.x) < 0.0001f)
        signX = 1.0f; // 0이면 기본 오른쪽으로


    v.x = signX * minXSpeed;
}
static void BreakGimmickLoop(Float2& v)
{
    // 수평 방향은 유지
    // vy만 크게 흔들어서 루프 탈출
    if (v.y >= 0.0f) v.y = -std::abs(v.y) - 8.0f;
    else             v.y = std::abs(v.y) + 8.0f;

    ClampBallSpeed(v, 20.0f, 42.0f);
    EnsureMinHorizontalSpeed(v, 8.0f);
}


static void ApplyBumperEffect(Float2& vel, const Float2& pos, const Int2& tile)
{
    float randomOffset = ((std::rand() % 100) / 100.0f - 0.5f) * 6.0f;

    // 수평 방향은 유지
    vel.y += randomOffset;

    // 약간 가속
    vel.x *= 1.08f;
    vel.y *= 1.08f;

    ClampBallSpeed(vel, 20.0f, 45.0f);
    EnsureMinHorizontalSpeed(vel, 10.0f);
}

void Ball::Reset(const Float2& p, const Float2& dir, float speed)
{
    pos = p;
    prevPos = p;
    prevTile = { -999, -999 };
    vel = { dir.x * speed, dir.y * speed };
	wallBounceStreak = 0;
}

void Ball::Tick(float deltaTime, const TileMap& map)
{
    prevPos = pos;

    if (map.GetWidth() <= 0 || map.GetHeight() <= 0)
    {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;
        return;
    }

    Float2 old = pos;

    pos.x += vel.x * deltaTime;
    pos.y += vel.y * deltaTime;

    ResolveTileCollision(map, old);

    Int2 curT = map.WorldToTile(pos);

    if (curT.x != prevTile.x || curT.y != prevTile.y)
    {
        TileId id = map.GetTile(curT.x, curT.y);
        auto props = map.GetProps(id);

        bool interactedWithGimmick = false;

        // 1) 범퍼는 통과형 트리거
        if (id == TileId::Bumper)
        {
            ApplyBumperEffect(vel, pos, curT);
            interactedWithGimmick = true;
        }
        // 2) 스티키 / 바람 계열
        else if (!props.isSolid)
        {
            if (props.speedMul != 1.0f ||
                props.force.x != 0.0f ||
                props.force.y != 0.0f)
            {
                vel.x *= props.speedMul;
                vel.y *= props.speedMul;

                vel.x += props.force.x;
                vel.y += props.force.y;

                interactedWithGimmick = true;
            }
        }

        // 3) 기믹과 상호작용했으면 루프 카운트 증가
        if (interactedWithGimmick)
        {
            gimmickLoopStreak++;

            if (gimmickLoopStreak >= 6)
            {
                BreakGimmickLoop(vel);
                gimmickLoopStreak = 0;
            }
        }

        ClampBallSpeed(vel, 18.0f, 40.0f);
        EnsureMinHorizontalSpeed(vel, 10.0f);

        prevTile = curT;
    }

    // Pong용 상/하 안전벽
    const float top = 2.0f + radius;
    const float bottom = (float)(map.GetHeight() - 1) - radius;

    if (pos.y < top)
    {
        pos.y = top;
        vel.y = std::abs(vel.y);

        gimmickLoopStreak++;
        if (gimmickLoopStreak >= 6)
        {
            BreakGimmickLoop(vel);
            gimmickLoopStreak = 0;
        }
    }
    else if (pos.y > bottom)
    {
        pos.y = bottom;
        vel.y = -std::abs(vel.y);

        gimmickLoopStreak++;
        if (gimmickLoopStreak >= 6)
        {
            BreakGimmickLoop(vel);
            gimmickLoopStreak = 0;
        }
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
    {
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

            float dx = std::abs(pos.x - oldPos.x);
            float dy = std::abs(pos.y - oldPos.y);

            const float pushEps = 0.05f;

            // 위 / 아래 바닥 타일인지 확인
            bool isTopOrBottomWall =
                (ty == 2) || (ty == map.GetHeight() - 1);
            // 2는 네 uiTopRows 기준 플레이 시작 벽 줄.
            // 나중에 uiTopRows를 멤버로 빼면 더 좋음.

            if (isTopOrBottomWall)
            {
                // 무조건 y만 반전
                pos.y = oldPos.y;
                vel.y = -vel.y * props.restitution;
                pos.y += (vel.y > 0.0f) ? pushEps : -pushEps;
            }
            else
            {
                // 나머지 내부 벽은 기존 축 판정 유지
                if (dx >= dy)
                {
                    pos.x = oldPos.x;
                    vel.x = -vel.x * props.restitution;
                    pos.x += (vel.x > 0.0f) ? pushEps : -pushEps;
                }
                else
                {
                    pos.y = oldPos.y;
                    vel.y = -vel.y * props.restitution;
                    pos.y += (vel.y > 0.0f) ? pushEps : -pushEps;
                }
            }

            gimmickLoopStreak++;

            ClampBallSpeed(vel, 18.0f, 40.0f);
            EnsureMinHorizontalSpeed(vel, 8.0f);
            return;
        }
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

            float angle = offset * (50.0f * 3.141592f / 180.0f);
            vel.x = +std::cos(angle) * speed;
            vel.y = std::sin(angle) * speed;

            // 패들 이동 영향 추가.
			vel.y += p.GetVelY() * 0.25f;

            float minXSpeed = 12.0f;
            if (std::abs(vel.x) < minXSpeed)
                vel.x = +minXSpeed;

            wallBounceStreak = 0;

            ClampBallSpeed(vel, 18.0f, 40.0f);
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

            float angle = offset * (50.0f * 3.141592f / 180.0f);
            vel.x = -std::cos(angle) * speed;
            vel.y = std::sin(angle) * speed;

            // 패들 이동 영향 추가.
            vel.y += p.GetVelY() * 0.25f;

            float minXSpeed = 12.0f;
            if (std::abs(vel.x) < minXSpeed)
                vel.x = -minXSpeed;

            wallBounceStreak = 0;

            ClampBallSpeed(vel, 18.0f, 40.0f);
        }
    }

}