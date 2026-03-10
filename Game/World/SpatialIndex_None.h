#pragma once
#include "ISpatialIndex.h"
#include <vector>

class SpatialIndex_None : public ISpatialIndex
{
public :
	void Clear() override;
	void InsertObstacle(Obstacle* obstacle) override;
	void InsertPowerUp(PowerUp* powerUp) override;
	void Query(const QueryAABB& area, SpatialQueryResult& outResult) override;


private :
	std::vector<Obstacle*> allObstacles;
	std::vector<PowerUp*> allPowerUps;
};

