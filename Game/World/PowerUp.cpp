#include "PowerUp.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Math/Color.h"
#include "Common/Types.h"


PowerUp::PowerUp(const Float2& pos, PowerUpType type)
	: position(pos), size(2.0f, 1.0f), type(type), active(true)
{
}

void PowerUp::Tick(float deltaTime)
{
	// 현재의 고정 아이템
}

void PowerUp::Draw()
{
	if (!active)
	{
		return;
	}

	const char* symbol = "?";

	switch (type)
	{
	case PowerUpType::MultiBall :
		symbol = "M";
		break;
		
	case PowerUpType::PaddleExpand :
		symbol = "P";
		break;

	case PowerUpType::BallSpeedUp :
		symbol = "S";
		break;
	}

	Renderer::Get().Submit(symbol, Vector2(static_cast<int>(position.x), static_cast<int>(position.y)), Color::White, 10 );
}
