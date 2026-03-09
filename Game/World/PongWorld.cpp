#include "PongWorld.h"
#include "Engine/Engine.h"
#include "Core/Input.h"
#include "Common/Types.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Game/GameManager.h"

#include "AI/SimpleTrackerAI.h"
#include "AI/AStarPositioningAI.h"

namespace KhyPong
{
	bool PongWorld::Init()
	{
		// 엔진 설정값 기준으로 월드 사이즈 결정.
        // 콘솔 화면 크기 기준으로 월드 크기 잡기 (문자 칸 단위).
        worldW = (float)Engine::Get().GetWidth();
        worldH = (float)Engine::Get().GetHeight();

        // 카메라 오프셋 0으로
        Renderer::Get().SetCameraOffset(Vector2::Zero);

        // 타일맵은 콘솔 기준이면 tileSize도 "문자 칸" 단위로 잡는 게 편함.
        map.SetTileSize(1); // 일단 1칸=1타일로 시작 추천 (나중에 2,4로 키워도 됨).

        // UI 상단 2줄 제외한 플레이 영역만.
        const int uiTopRows = 2;

        // csv 맵로드
        const int mapW = (int)worldW;
        const int mapH = (int)worldH;

        map.FillTestMap(mapW, mapH, uiTopRows);

		// 패들 초기화.
		left.Reset({ 2.0f, worldH * 0.5f});
		right.Reset({ worldW - 3.0f, worldH * 0.5f });

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
        Float2 dir{ (float)serveDir, 0.35f };

		ball.Reset(center, dir, 30.0f);

        left.Reset({ 2.0f,  worldH * 0.5f });
        right.Reset({ worldW - 3.0f, worldH * 0.5f });

		waitingServe = false;
		serveTimer = 0.0f;
	}

	void PongWorld::Tick(float deltaTime)
	{
		HandleInput(deltaTime);
		UpdateGameplay(deltaTime);
		HandleSecoreAndReset(deltaTime);
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

        // F2로 타일 그리드.
        if (Input::Get().GetKeyDown(VK_F2)) dbgGrid = !dbgGrid;

		// F3으로 존 표시.
        if (Input::Get().GetKeyDown(VK_F3)) dbgZones = !dbgZones;

		// F4공 AABB/검사 영역.
        if (Input::Get().GetKeyDown(VK_F4)) dbgBallBox = !dbgBallBox;
    }

    void PongWorld::UpdateGameplay(float dt)
    {
        if (matchEnded) return; // 경기 끝나면 움직임 중지

        // 패들 업데이트(클램프 포함)
        left.Tick(dt, worldH);
        right.Tick(dt, worldH);

        
        if (!waitingServe)
        {
            // 공 업데이트 + 타일 충돌
            ball.Tick(dt, map);
            // Step A니까 월드 반사 버전으로!
            //ball.Tick(dt, worldW, worldH);
            //ApplyZonesToBall(dt);
            ball.ResolvePaddleCollision(left);
            ball.ResolvePaddleCollision(right);
        }
    }

    
    void PongWorld::HandleSecoreAndReset(float dt)
    {
        // 1. 경기 종료 상태
        if (matchEnded)
        {
            endTimer -= dt;
            if (endTimer <= 0.0f)
            {
                GameManager::Get().RequestGoMenu();
            }
            return;
        }

        // 2. 서브 대기 상태
        if (waitingServe)
        {
            serveTimer -= dt;
            if (serveTimer <= 0.0f)
            {
                ResetRound();
            }
            return;
        }

        // 3. 득점 판정
        bool scored = false;

        if (ball.GetPos().x < 0.0f)
        {
            rightScore++;
            serveDir = -1;
            scored = true;
        }
        else if (ball.GetPos().x > worldW - 1.0f)
        {
            leftScore++;
            serveDir = +1;
            scored = true;
        }

        if (!scored)
            return;

        // 4. 승리 판정
        if (leftScore >= scoreToWin || rightScore >= scoreToWin)
        {
            matchEnded = true;
            endTimer = 3.0f;
            leftWon = (leftScore >= scoreToWin);

            waitingServe = false;
            ball.Reset({ worldW * 0.5f, worldH * 0.5f }, { 0.0f, 0.0f }, 0.0f);
            return;
        }

        // 5. 다음 라운드 준비
        waitingServe = true;
        serveTimer = 0.7f;
    }

    void PongWorld::Draw()
    {
        Renderer::Get().SetCameraOffset(Vector2::Zero);

        // 점수
        static char scoreBuf[64];
        sprintf_s(scoreBuf, "L:%d  R:%d", leftScore, rightScore);
        Renderer::Get().Submit(scoreBuf, Vector2(2, 1), Color::White, 50);

        static char dbg[128];
        Int2 t = map.WorldToTile(ball.GetPos());
        sprintf_s(dbg, "tile(%d,%d) id=%d vx=%.2f vy=%.2f",
            t.x, t.y, (int)map.GetTile(t.x, t.y),
            ball.GetVel().x, ball.GetVel().y);
        Renderer::Get().Submit(dbg, Vector2(2, 3), Color::White, 200);

        // 중앙선(점선)
        for (int y = 0; y < (int)worldH; ++y)
            if (y % 2 == 0)
                Renderer::Get().Submit(":", Vector2((int)(worldW * 0.5f), y), Color::Blue, 0);

        left.Draw();
        right.Draw();

        // 공
        int bx = (int)std::round(ball.GetPos().x);
        int by = (int)std::round(ball.GetPos().y);
        Renderer::Get().Submit("O", Vector2(bx, by), Color::Yellow, 100);

        // READY
        if (waitingServe)
            Renderer::Get().Submit("READY...", Vector2((int)(worldW * 0.5f - 4), (int)(worldH * 0.5f)), Color::White, 60);

        // WIN/LOSE(3초)
        if (matchEnded)
        {
            const char* msg = leftWon ? "LEFT WIN!" : "RIGHT WIN!";
            Renderer::Get().Submit(msg,
                Vector2((int)(worldW * 0.5f - 5), (int)(worldH * 0.5f - 1)),
                Color::Yellow, 100);

            static char tbuf[32];
            sprintf_s(tbuf, "Back to menu in %.1f", endTimer);
            Renderer::Get().Submit(tbuf,
                Vector2((int)(worldW * 0.5f - 9), (int)(worldH * 0.5f + 1)),
                Color::White, 100);
        }
        
        DrawDebug();
    }

    void PongWorld::DrawDebug()
    {
        // 1) 그리드: 타일 경계 느낌(점선)
        if (dbgGrid)
        {
            const int w = (int)worldW;
            const int h = (int)worldH;

            // 가로줄(예: 4칸마다)
            for (int y = 0; y < h; y += 4)
                for (int x = 0; x < w; ++x)
                    Renderer::Get().Submit(".", Vector2(x, y), Color::Green, 70);

            // 세로줄(예: 8칸마다)
            for (int x = 0; x < w; x += 8)
                for (int y = 0; y < h; ++y)
                    Renderer::Get().Submit(".", Vector2(x, y), Color::Green, 70);
        }

        // 2) 존 표시(슬로우/스피드).
        if (dbgZones)
        {
            DrawZonesDebug();
        }

        // 3) 공 AABB.
        if (dbgBallBox)
        {
            int x0 = (int)std::round(ball.GetPos().x - 1);
            int x1 = (int)std::round(ball.GetPos().x + 1);
            int y0 = (int)std::round(ball.GetPos().y - 1);
            int y1 = (int)std::round(ball.GetPos().y + 1);

            for (int x = x0; x <= x1; ++x)
            {
                Renderer::Get().Submit("*", Vector2(x, y0), Color::Red, 80);
                Renderer::Get().Submit("*", Vector2(x, y1), Color::Red, 80);
            }
            for (int y = y0; y <= y1; ++y)
            {
                Renderer::Get().Submit("*", Vector2(x0, y), Color::Red, 80);
                Renderer::Get().Submit("*", Vector2(x1, y), Color::Red, 80);
            }
        }
    }

    void PongWorld::DrawZonesDebug()
    {
        const int w = map.GetWidth();
        const int h = map.GetHeight();
        if (w <= 0 || h <= 0) return;

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                TileId id = map.GetTile(x, y);

                if (id == TileId::SolidWall)
                    Renderer::Get().Submit("#", Vector2(x, y), Color::White, 80);
                else if (id == TileId::SlowZone)
                    Renderer::Get().Submit("s", Vector2(x, y), Color::Blue, 80);
                else if (id == TileId::SpeedZone)
                    Renderer::Get().Submit("f", Vector2(x, y), Color::Green, 80);
            }
    }

    void PongWorld::ApplyZonesToBall(float deltaTime)
    {
        {
            // 공이 존에 들어갈 때만 '한 번' 적용하면 제일 좋지만,
            // Step A에서는 간단히 "들어가 있는 동안" 약하게 적용하자.
            int bx = (int)std::round(ball.GetPos().x);
            int by = (int)std::round(ball.GetPos().y);

            for (const auto& z : zones)
            {
                if (bx >= z.x0 && bx <= z.x1 && by >= z.y0 && by <= z.y1)
                {
                    // 너무 과하게 누적되면 속도가 폭주하니까, dt 기반으로 아주 약하게
                    Float2 v = ball.GetVel();
                    v.x *= std::pow(z.speedMul, deltaTime * 6.0f); // dt 보정 (대충 6은 체감 튜닝값)
                    v.y *= std::pow(z.speedMul, deltaTime * 6.0f);
                    ball.SetVel(v); // Ball에 setter 추가 필요(아래 참고)
                    return;
                }
            }
        }
    }

}
