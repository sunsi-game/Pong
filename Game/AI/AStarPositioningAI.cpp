#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include "AStarPositioningAI.h"


static float PredictBallYAtX(
    const TileMap& map,
    const Float2& ballPos,
    const Float2& ballVel,
    float targetX)
{
    if (std::abs(ballVel.x) < 0.0001f)
        return ballPos.y;

    float t = (targetX - ballPos.x) / ballVel.x;
    if (t < 0.0f)
        return ballPos.y;

    float predictedY = ballPos.y + ballVel.y * t;

    // 상/하 벽 반사 단순 보정
    const float top = 2.0f;
    const float bottom = (float)(map.GetHeight() - 1);

    // 벽 반사를 "접어서" 근사
    float height = bottom - top;
    if (height <= 0.0f) return ballPos.y;

    while (predictedY < top || predictedY > bottom)
    {
        if (predictedY < top)
            predictedY = top + (top - predictedY);
        else if (predictedY > bottom)
            predictedY = bottom - (predictedY - bottom);
    }

    return predictedY;
}

struct AStarNode
{
    Int2 pos;
    float g = 0.0f;
    float h = 0.0f;
    int parentIndex = -1;
    bool closed = false;

    float F() const { return g + h; }
};

static float Heuristic(const Int2& a, const Int2& b)
{
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

static bool IsInsideMap(const TileMap& map, const Int2& p)
{
    return p.x >= 0 && p.x < map.GetWidth() &&
        p.y >= 0 && p.y < map.GetHeight();
}

static bool RunAStar(
    const TileMap& map,
    const Int2& start,
    const Int2& goal,
    std::vector<Int2>& outPath)
{
    outPath.clear();

    if (!IsInsideMap(map, start) || !IsInsideMap(map, goal))
        return false;

    TileProps goalProps = map.GetProps(map.GetTile(goal.x, goal.y));
    if (goalProps.isSolid)
        return false;

    const int width = map.GetWidth();
    const int height = map.GetHeight();
    const int nodeCount = width * height;

    auto ToIndex = [width](const Int2& p)
        {
            return p.y * width + p.x;
        };

    std::vector<AStarNode> nodes(nodeCount);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = y * width + x;
            nodes[idx].pos = { x, y };
            nodes[idx].g = std::numeric_limits<float>::infinity();
            nodes[idx].h = 0.0f;
            nodes[idx].parentIndex = -1;
            nodes[idx].closed = false;
        }
    }

    struct PQItem
    {
        int index;
        float f;

        bool operator<(const PQItem& other) const
        {
            return f > other.f;
        }
    };

    std::priority_queue<PQItem> openSet;

    int startIndex = ToIndex(start);
    int goalIndex = ToIndex(goal);

    nodes[startIndex].g = 0.0f;
    nodes[startIndex].h = Heuristic(start, goal);

    openSet.push({ startIndex, nodes[startIndex].F() });

    const Int2 directions[4] =
    {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };

    while (!openSet.empty())
    {
        int currentIndex = openSet.top().index;
        openSet.pop();

        AStarNode& current = nodes[currentIndex];

        if (current.closed)
            continue;

        current.closed = true;

        if (currentIndex == goalIndex)
        {
            std::vector<Int2> reversedPath;

            int trace = goalIndex;
            while (trace != -1)
            {
                reversedPath.push_back(nodes[trace].pos);
                trace = nodes[trace].parentIndex;
            }

            std::reverse(reversedPath.begin(), reversedPath.end());
            outPath = std::move(reversedPath);
            return true;
        }

        for (const Int2& dir : directions)
        {
            Int2 nextPos{ current.pos.x + dir.x, current.pos.y + dir.y };

            if (!IsInsideMap(map, nextPos))
                continue;

            int nextIndex = ToIndex(nextPos);
            AStarNode& nextNode = nodes[nextIndex];

            if (nextNode.closed)
                continue;

            TileId nextTile = map.GetTile(nextPos.x, nextPos.y);
            TileProps props = map.GetProps(nextTile);

            if (props.isSolid)
                continue;

            float moveCost = 1.0f + static_cast<float>(props.extraCost);
            float tentativeG = current.g + moveCost;

            if (tentativeG < nextNode.g)
            {
                nextNode.g = tentativeG;
                nextNode.h = Heuristic(nextPos, goal);
                nextNode.parentIndex = currentIndex;

                openSet.push({ nextIndex, nextNode.F() });
            }
        }
    }

    return false;
}

PaddleInput AStarPositioningAI::Update(
    float dt,
    const TileMap& map,
    const Float2& paddlePos,
    const Float2& ballPos,
    const Float2& ballVel)
{
    PaddleInput in{};

    float targetY = map.GetHeight() * 0.5f;

    // 공이 오른쪽 AI 쪽으로 오는 중이면 추적
    if (ballVel.x > 0.0f)
    {
        targetY = ballPos.y;
    }

    Int2 start = map.WorldToTile(paddlePos);
    Int2 goal = map.WorldToTile({ paddlePos.x, targetY });

    std::vector<Int2> livePath;
    if (RunAStar(map, start, goal, livePath))
    {
        if (livePath.size() >= 2)
        {
            Int2 next = livePath[1];

            if (next.y > start.y) in.moveY = 1.0f;
            else if (next.y < start.y) in.moveY = -1.0f;
            else in.moveY = 0.0f;
        }
    }
    else
    {
        if (targetY > paddlePos.y + 0.5f) in.moveY = 1.0f;
        else if (targetY < paddlePos.y - 0.5f) in.moveY = -1.0f;
        else in.moveY = 0.0f;
    }

    debugPathRefreshTimer += dt;
    if (debugPathRefreshTimer >= debugPathRefreshInterval)
    {
        debugPathRefreshTimer = 0.0f;
        debugPath = livePath;
    }

    return in;
}