#pragma once

#include "Level/BaseLevel.h"
#include "Math/Vector2.h"

using namespace KhyPong;

//전방 선언.
namespace KhyPong{ class PongWorld; }


class GameLevel : public BaseLevel
{
	// RTTI 등록.
	RTTI_DECLARATIONS(GameLevel, Level)

public :
	GameLevel();
	virtual ~GameLevel();

	void BeginPlay() override;
	void Tick(float deltaTime) override;
	void Draw() override;

private:
	PongWorld* world = nullptr;
	bool started = false;

};
