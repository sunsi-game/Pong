#pragma once
#include <vector>

class Obstacle;
class PowerUp;

struct QueryAABB
{
    float left;
    float top;
    float right;
    float bottom;
};

struct SpatialQueryResult
{
    std::vector<Obstacle*> obstacles;
    std::vector<PowerUp*> powerUps;
};

class ISpatialIndex
{
public:
    virtual ~ISpatialIndex() = default;

    virtual void Clear() = 0;
    virtual void InsertObstacle(Obstacle* obstacle) = 0;
    virtual void InsertPowerUp(PowerUp* powerUp) = 0;
    virtual void Query(const QueryAABB& area, SpatialQueryResult& outResult) = 0;
};