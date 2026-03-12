#pragma once
#include "Actor/Actor.h"
#include "Math/Vector2.h"
#include "World/PongCollisionTypes.h"
#include "World/SpatialMath.h"
#include "Common/Types.h"
#include "Render/Renderer.h"

using namespace KhyPong;

enum class PowerUpType
{
	MultiBall,
	PaddleExpand,
	BallSpeedUp
};

class PowerUp : Actor
{
public:
    PowerUp() = default;

    PowerUp(const Float2& p, PowerUpType t);

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

    inline PowerUpType GetTypes() const { return type; }
    inline bool IsActive() const { return active; }
    inline void Consume() { active = false; }

    void Draw() const
    {
        if (!active) return;

        const char* symbol = "?";
        switch (type)
        {
        case PowerUpType::MultiBall:   symbol = "M"; break;
        case PowerUpType::PaddleExpand:symbol = "P"; break;
        case PowerUpType::BallSpeedUp: symbol = "S"; break;
        }

        Renderer::Get().Submit(symbol, Vector2((int)position.x, (int)position.y), Color::Yellow, 40);
    }

private:
    Float2 position{ 0.0f, 0.0f };
    Float2 size{ 2.0f, 1.0f };
	PowerUpType type = PowerUpType::MultiBall;
	bool active;
};