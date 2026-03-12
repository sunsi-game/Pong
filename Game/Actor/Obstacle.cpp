#include "Obstacle.h"
#include "Render/Renderer.h"
#include "Math/Color.h"

Obstacle::Obstacle()
    : position(0.0f, 0.0f), size(4.0f, 2.0f)
{
}

Obstacle::Obstacle(const Float2& pos, const Float2& size)
    : position(pos), size(size)
{
}

void Obstacle::Tick(float deltaTime)
{
    // 정적 오브젝트라 현재는 아무 것도 안함.
}

void Obstacle::Draw()
{
    for (int y = 0; y < static_cast<int>(size.y); ++y)
    {
        for (int x = 0; x < static_cast<int>(size.x); ++x)
        {
            Renderer::Get().Submit(
                "#",
                Vector2(static_cast<int>(position.x) + x, static_cast<int>(position.y) + y),
                Color::White,
                10
            );
        }
    }
}
