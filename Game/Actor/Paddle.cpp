#include "Paddle.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"

#include <algorithm>

using namespace KhyPong;

static float Clamp(float v, float mn, float mx)
{
	return (std::max)(mn, (std::min)(mx, v));
}

void Paddle::Tick(float deltaTime, float worldH)
{
	// 입력에 따른 속도 변경.
	velY = moveInput * speed * deltaTime;
	// 위치 업데이트.
	pos.y += velY;
	
	// 월드 경계 내로 위치 제한.
	float minY = halfH;
	float maxY = worldH - 1.0f - halfH;
	if (pos.y < minY) pos.y = minY;
	if (pos.y > maxY) pos.y = maxY;
}
void Paddle::Draw()
{
	float x = pos.x;
	int y0 = (int)(pos.y - halfH);
	int y1 = (int)(pos.y + halfH);

	for (int y = y0; y <= y1; ++y)
		Renderer::Get().Submit("|", Vector2(x, y), Color::White,10);
}