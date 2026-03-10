#pragma once
#include "Actor/Actor.h"
#include "PongCollisionTypes.h"
#include "SpatialMath.h"
#include "Common/Types.h"
#include "Render/Renderer.h"

using namespace KhyPong;

class Obstacle : public Actor
{
public :
	Obstacle();
    Obstacle(const Float2& p, const Float2& s);

    inline const Float2& GetPos() const { return position; }
    inline const Float2& GetSize() const { return size; }

    void Tick(float deltaTime);
    void Draw();

    inline float GetLeft()   const { return position.x; }
    inline float GetTop()    const { return position.y; }
    inline float GetRight()  const { return position.x + size.x; }
    inline float GetBottom() const { return position.y + size.y; }

    RectF GetBounds() const
    {
        return RectF{ GetLeft(), GetTop(), GetRight(), GetBottom() };
    }

    void Draw() const
    {
        const int x0 = (int)position.x;
        const int y0 = (int)position.y;
        const int w = (int)size.x;
        const int h = (int)size.y;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                Renderer::Get().Submit("#", Vector2(x0 + x, y0 + y), Color::White, 30);
            }
        }
    }

private:
    Float2 position{ 0.0f, 0.0f };
    Float2 size{ 1.0f, 1.0f };
};