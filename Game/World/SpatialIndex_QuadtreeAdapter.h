#pragma once
#include "ISpatialIndex.h"
#include "SpatialMath.h"

#include <array>
#include <memory>
#include <vector>

class Obstacle;
class PowerUp;

class SpatialIndex_QuadtreeAdapter : public ISpatialIndex
{
public:
    SpatialIndex_QuadtreeAdapter(float worldWidth, float worldHeight, int maxDepth = 5, int capacity = 4);
    ~SpatialIndex_QuadtreeAdapter() override = default;

    void Clear() override;
    void InsertObstacle(Obstacle* obstacle) override;
    void InsertPowerUp(PowerUp* powerUp) override;
    void Query(const QueryAABB& area, SpatialQueryResult& outResult) override;

private:
    struct Node
    {
        RectF bounds;
        int depth = 0;

        std::vector<Obstacle*> obstacles;
        std::vector<PowerUp*> powerUps;

        std::array<std::unique_ptr<Node>, 4> children;

        bool IsLeaf() const
        {
            return children[0] == nullptr;
        }
    };

private:
    void Subdivide(Node& node);
    bool InsertObstacle(Node& node, Obstacle* obstacle);
    bool InsertPowerUp(Node& node, PowerUp* powerUp);
    void QueryNode(Node& node, const RectF& area, SpatialQueryResult& outResult);

    RectF ToRect(const QueryAABB& area) const;

private:
    std::unique_ptr<Node> root;
    float worldWidth = 0.0f;
    float worldHeight = 0.0f;
    int maxDepth = 5;
    int capacity = 4;
};