#include "Paddle.h"
#include <algorithm>

static float Clamp(float v, float min, float max)
{
	return std::max(min, std::min(max, v));
}

void Paddle::Tick(float deltaTime, float worldH)
{
	// 입력에 따른 속도 변경.
	velY = moveInput * speed;
	// 위치 업데이트.
	pos.y += velY * deltaTime;
	
	// 월드 경계 내로 위치 제한.
	float minY = halfH;
	float maxY = worldH - halfH;
	pos.y = Clamp(pos.y, minY, maxY);
}