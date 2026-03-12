#include "SpatialIndex_None.h"
#include "Actor/Obstacle.h"
#include "Actor/PowerUp.h"

void SpatialIndex_None::Clear()
{
	allObstacles.clear();
	allPowerUps.clear();
}

void SpatialIndex_None::InsertObstacle(Obstacle* obstacle)
{
	allObstacles.push_back(obstacle);
}

void SpatialIndex_None::InsertPowerUp(PowerUp* powerUp)
{
	allPowerUps.push_back(powerUp);
}

void SpatialIndex_None::Query(const QueryAABB& area, SpatialQueryResult& outResult)
{
	outResult.obstacles = allObstacles;
	outResult.powerUps = allPowerUps;
}
