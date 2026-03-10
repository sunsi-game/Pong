#include "SpatialIndex_QuadtreeAdapter.h"
#include "Obstacle.h"
#include "PowerUp.h"

SpatialIndex_QuadtreeAdapter::SpatialIndex_QuadtreeAdapter(float inWorldWidth, float inWorldHeight, int inMaxDepth, int inCapacity)
    : worldWidth(inWorldWidth), worldHeight(inWorldHeight), maxDepth(inMaxDepth), capacity(inCapacity)
{
    Clear();
}

void SpatialIndex_QuadtreeAdapter::Clear()
{
    root = std::make_unique<Node>();
    root->bounds = RectF{ 0.0f, 0.0f, worldWidth, worldHeight };
    root->depth = 0;
}

RectF SpatialIndex_QuadtreeAdapter::ToRect(const QueryAABB& area) const
{
    return RectF{ area.left, area.top, area.right, area.bottom };
}

void SpatialIndex_QuadtreeAdapter::Subdivide(Node& node)
{
    const float midX = (node.bounds.left + node.bounds.right) * 0.5f;
    const float midY = (node.bounds.top + node.bounds.bottom) * 0.5f;

    // 0: LT, 1: RT, 2: LB, 3: RB
    node.children[0] = std::make_unique<Node>();
    node.children[0]->bounds = RectF{ node.bounds.left, node.bounds.top, midX, midY };
    node.children[0]->depth = node.depth + 1;

    node.children[1] = std::make_unique<Node>();
    node.children[1]->bounds = RectF{ midX, node.bounds.top, node.bounds.right, midY };
    node.children[1]->depth = node.depth + 1;

    node.children[2] = std::make_unique<Node>();
    node.children[2]->bounds = RectF{ node.bounds.left, midY, midX, node.bounds.bottom };
    node.children[2]->depth = node.depth + 1;

    node.children[3] = std::make_unique<Node>();
    node.children[3]->bounds = RectF{ midX, midY, node.bounds.right, node.bounds.bottom };
    node.children[3]->depth = node.depth + 1;
}

void SpatialIndex_QuadtreeAdapter::InsertObstacle(Obstacle* obstacle)
{
    if (obstacle == nullptr)
        return;

    InsertObstacle(*root, obstacle);
}

void SpatialIndex_QuadtreeAdapter::InsertPowerUp(PowerUp* powerUp)
{
    if (powerUp == nullptr)
        return;

    InsertPowerUp(*root, powerUp);
}

bool SpatialIndex_QuadtreeAdapter::InsertObstacle(Node& node, Obstacle* obstacle)
{
    const RectF objBounds = obstacle->GetBounds();

    if (!Intersects(node.bounds, objBounds))
        return false;

    if (node.depth >= maxDepth)
    {
        node.obstacles.push_back(obstacle);
        return true;
    }

    if (node.IsLeaf() && static_cast<int>(node.obstacles.size() + node.powerUps.size()) >= capacity)
    {
        Subdivide(node);
    }

    if (!node.IsLeaf())
    {
        for (auto& child : node.children)
        {
            if (Contains(child->bounds, objBounds))
            {
                return InsertObstacle(*child, obstacle);
            }
        }
    }

    node.obstacles.push_back(obstacle);
    return true;
}

bool SpatialIndex_QuadtreeAdapter::InsertPowerUp(Node& node, PowerUp* powerUp)
{
    const RectF objBounds = powerUp->GetBounds();

    if (!Intersects(node.bounds, objBounds))
        return false;

    if (node.depth >= maxDepth)
    {
        node.powerUps.push_back(powerUp);
        return true;
    }

    if (node.IsLeaf() && static_cast<int>(node.obstacles.size() + node.powerUps.size()) >= capacity)
    {
        Subdivide(node);
    }

    if (!node.IsLeaf())
    {
        for (auto& child : node.children)
        {
            if (Contains(child->bounds, objBounds))
            {
                return InsertPowerUp(*child, powerUp);
            }
        }
    }

    node.powerUps.push_back(powerUp);
    return true;
}

void SpatialIndex_QuadtreeAdapter::Query(const QueryAABB& area, SpatialQueryResult& outResult)
{
    outResult.obstacles.clear();
    outResult.powerUps.clear();

    const RectF queryRect = ToRect(area);
    QueryNode(*root, queryRect, outResult);
}

void SpatialIndex_QuadtreeAdapter::QueryNode(Node& node, const RectF& area, SpatialQueryResult& outResult)
{
    if (!Intersects(node.bounds, area))
        return;

    for (Obstacle* obstacle : node.obstacles)
    {
        if (obstacle && Intersects(obstacle->GetBounds(), area))
        {
            outResult.obstacles.push_back(obstacle);
        }
    }

    for (PowerUp* powerUp : node.powerUps)
    {
        if (powerUp && powerUp->IsActive() && Intersects(powerUp->GetBounds(), area))
        {
            outResult.powerUps.push_back(powerUp);
        }
    }

    if (node.IsLeaf())
        return;

    for (auto& child : node.children)
    {
        if (child)
        {
            QueryNode(*child, area, outResult);
        }
    }
}