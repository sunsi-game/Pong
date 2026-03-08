#include "PongWorld.h"
#include "Engine/Engine.h"
#include "Core/Input.h"
#include "Common/Types.h"

#include "AI/SimpleTrackerAI.h"
#include "AI/AStarPositioningAI.h"

namespace KhyPong
{
	bool PongWorld::Init()
	{
		// 엔진 설정값 기준으로 월드 사이즈 결정.
		worldW = 256.0f;
		worldH = 144.0f;

		// 타일맵 로드.
		map.SetTileSize(16.0f);
		// map.LoadFromFile("Assets/Map/pong_map.txt"); // 확장 모드 자리.

		// 패들 초기화.
		left.Reset({ 24.0f, worldH * 0.5f});
		right.Reset({ worldW - 24.0f, worldH * 0.5f });

		// AI 선택.
		if (useAStarAI)
			rightAI = std::make_unique<AStarPositioningAI>();
		else
			rightAI = std::make_unique<SimpleTrackerAI>();

		return true;
	}

	inline void PongWorld::ResetRound()
	{
		// 공 중앙 스폰.
		Float2 center{ worldW * 0.5f, worldH * 0.5f };

		// 오른쪽 위로 쏘는 기본 방향.
		Float2 dir{ 1.0f, 0.35f };
		ball.Reset(center, dir, 140.0f);

		left.Reset({ 24.0f, worldH * 0.5f });
		right.Reset({ worldW - 24.0f, worldH * 0.5f });
	}

	void PongWorld::Tick(float deltaTime)
	{
		HandleInput(deltaTime);
		UpdateGameplay(deltaTime);
		HandleSecoreAndReset();
	}

    void PongWorld::HandleInput(float dt)
    {
        // 왼쪽 패들 : 플레이어 입력.
        float move = 0.0f;

        // 위로.
        if (Input::Get().GetKey('W') || Input::Get().GetKey(VK_UP))
        {
            move -= 1.0f;
        }

        // 아래로.
        if (Input::Get().GetKey('S') || Input::Get().GetKey(VK_DOWN))
        {
            move += 1.0f;
        }

        left.SetMoveInput(move);

        // 오른쪽 패들 : AI.
        if (rightAI)
        {
            PaddleInput aiIn = rightAI->Update(dt, map, right.GetPos(), ball.GetPos(), ball.GetVel());
            right.SetMoveInput(aiIn.moveY);
        }

        // 라운드 리셋.
        if (Input::Get().GetKeyDown(VK_SPACE))
        {
            ResetRound();
        }

        // F1 눌러서 A* 전환.
        if (Input::Get().GetKeyDown(VK_F1))
        {
            useAStarAI = !useAStarAI;
            if (useAStarAI)
            {
                rightAI = std::make_unique<AStarPositioningAI>();
            }
            else
            {
                rightAI = std::make_unique<SimpleTrackerAI>();
            }
        }
    }

    void PongWorld::UpdateGameplay(float dt)
    {
        // 패들 업데이트(클램프 포함)
        left.Tick(dt, worldH);
        right.Tick(dt, worldH);

        // 공 업데이트 + 타일 충돌
        ball.Tick(dt, map);

        // 공-패들 충돌
        ball.ResolvePaddleCollision(left);
        ball.ResolvePaddleCollision(right);
    }

    
    void PongWorld::HandleSecoreAndReset()
    {
        // 골 판정: 좌/우 바깥으로 나가면 점수
        if (ball.GetPos().x < 0.0f)
        {
            rightScore++;
            ResetRound();
        }
        else if (ball.GetPos().x > worldW)
        {
            leftScore++;
            ResetRound();
        }

        // 승리 조건
        if (leftScore >= scoreToWin || rightScore >= scoreToWin)
        {
            leftScore = 0;
            rightScore = 0;
            ResetRound();
        }
    }

    void PongWorld::Draw()
    {
        // 여기서 렌더러에 그리기 요청을 넣으면 됨.
        // 너 엔진 Renderer API에 맞게 바꿔.

        // 디버그용으로 타일, 패들, 공을 그리자.
        // (예시) renderer->DrawRect / DrawText / DrawChar 등등
        // Renderer* r = Engine::Get().GetRenderer();

        // 1) 타일맵 그리기
        // map.DebugDraw(*r);

        // 2) 엔티티 그리기
        // left.Draw(*r);
        // right.Draw(*r);
        // ball.Draw(*r);

        // 3) 점수 표시
        // r->DrawText(...);
    }
}
