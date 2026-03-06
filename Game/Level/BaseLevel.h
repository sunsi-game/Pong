#pragma once
#include "Level/Level.h"
#include "Game/GameManager.h"

using namespace KhyPong;

class BaseLevel : public Level
{
    RTTI_DECLARATIONS(BaseLevel, Level)
public:
    virtual void Tick(float deltaTime) override
    {
        // 레벨이 살아있는 동안 GameManager를 매 프레임 처리.
        GameManager::Get().Tick();

        // 기존 레벨 로직
        Level::Tick(deltaTime);
    }

    virtual void Draw() override
    {
        Level::Draw();
    }
};

