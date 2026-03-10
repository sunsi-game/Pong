#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "PongWorld.h"
#include "Engine/Engine.h"
#include "Core/Input.h"
#include "Common/Types.h"
#include "Render/Renderer.h"
#include "Math/Vector2.h"
#include "Game/GameManager.h"
#include "Common/Common.h"

#include "AI/SimpleTrackerAI.h"
#include "AI/AStarPositioningAI.h"

#include "Obstacle.h"
#include "PowerUp.h"
#include "ISpatialIndex.h"
#include "SpatialIndex_None.h"
#include "SpatialIndex_QuadtreeAdapter.h"


namespace KhyPong
{
    PongWorld::~PongWorld() = default;

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
        left.Reset({ 2.0f, worldH * 0.5f });
        right.Reset({ worldW - 3.0f, worldH * 0.5f });

        // AI 선택.
        if (useAStarAI)
            rightAI = std::make_unique<AStarPositioningAI>();
        else
            rightAI = std::make_unique<SimpleTrackerAI>();

        spatialIndex = std::make_unique<SpatialIndex_None>();

        BuildStepDObjects();
        ResetRound();

        matchEnded = false;
        roundEnded = false;
        waitingServe = false;

        return true;
    }

    inline void PongWorld::ResetRound()
    {
        Float2 center{ worldW * 0.5f, worldH * 0.5f };

        // 각도 랜덤 (도 단위)
        float minAngle = 15.0f;
        float maxAngle = 45.0f;

        float angleDeg = minAngle +
            (std::rand() / (float)RAND_MAX) * (maxAngle - minAngle);

        // 위/아래 랜덤
        if (std::rand() % 2 == 0)
            angleDeg = -angleDeg;

        float angleRad = angleDeg * 3.14159265f / 180.0f;

        Float2 dir(0,0);
        dir.x = (float)serveDir * std::cos(angleRad);
        dir.y = std::sin(angleRad);

        ball.Reset(center, dir, 30.0f);
        ball.SetPos(center);
        balls.clear();
        balls.push_back(ball);


        left.SetPos({ 2.0f,  worldH * 0.5f });
        right.SetPos({ worldW - 3.0f, worldH * 0.5f });
        

        waitingServe = false;
        serveTimer = 0.0f;

        roundTimeLimit = 35.0f;
        roundTimeRemaining = 35.0f;
        roundEnded = false;
        timedMultiBallStage = 0;

        bruteForceChecks = 0;
        spatialCandidateChecks = 0;
        actualCollisionCount = 0;
        obstacleCandidateCount = 0;
        powerUpCandidateCount = 0;

        matchEnded = false;
        leftWon = false;
        endTimer = 0.0f;

        RebuildSpatialIndex();
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

        const Ball* targetBall = balls.empty() ? &ball : &balls.front();
        // 오른쪽 패들 : AI.
        if (rightAI && targetBall)
        {
            //PaddleInput aiIn = rightAI->Update(dt, map, right.GetPos(), ball.GetPos(), ball.GetVel());
            PaddleInput aiIn = rightAI->Update(dt, map, right.GetPos(), targetBall->GetPos(), targetBall->GetVel());
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

        if (Input::Get().GetKeyDown(VK_F5)) ToggleSpatialIndexMode();

        if (Input::Get().GetKeyDown('V')) dbgPath = !dbgPath;

    }

    void PongWorld::UpdateGameplay(float dt)
    {
        if (matchEnded) return; // 경기 끝나면 움직임 중지
        if (roundEnded) return;

        // 패들 업데이트(클램프 포함)
        left.Tick(dt, worldH);
        right.Tick(dt, worldH);

        UpdateRoundTimer(dt); // 추가.

        if (!waitingServe)
        {
            // 공 업데이트 + 타일 충돌
            //ball.Tick(dt, map);
            // Step A니까 월드 반사 버전으로!
            //ball.Tick(dt, worldW, worldH);
            //ApplyZonesToBall(dt);
            //ball.ResolvePaddleCollision(left);
            //ball.ResolvePaddleCollision(right);
            UpdateRoundTimer(dt);
            UpdateBalls(dt);
        }

        RebuildSpatialIndex();
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

        for (Ball& curBall : balls) {
            if (curBall.GetPos().x < 0.0f)
            {
                rightScore++;
                serveDir = -1;
                scored = true;
                break;
            }
            else if (curBall.GetPos().x > worldW - 1.0f)
            {
                leftScore++;
                serveDir = +1;
                scored = true;
                break;
            }
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
            balls.clear();
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
        sprintf_s(scoreBuf, "Player:%d  AI:%d", leftScore, rightScore);
        Renderer::Get().Submit(scoreBuf, Vector2(2, 1), Color::White, 50);
        char timeBuf[64];
        sprintf_s(timeBuf, "Time: %d", (int)roundTimeRemaining);
        Renderer::Get().Submit(timeBuf, Vector2(2, 3), Color::White, 50);


        // 키 옵션 안내.
        static char optionBuf[220];
        sprintf_s(optionBuf, "F1: AI %s | F2: Grid %s | F3: Zones %s | F4: BallBox %s | V: Path %s | F5: Spatial %s",
            useAStarAI ? "A*" : "Simple",
            dbgGrid ? "ON" : "OFF",
            dbgZones ? "ON" : "OFF",
            dbgBallBox ? "ON" : "OFF",
            dbgPath ? "ON" : "OFF",
            (spatialMode == SpatialIndexMode::None) ? "None" : "Quadtree");
        Renderer::Get().Submit(optionBuf, Vector2(2, 2), Color::White, 40);

        // 중앙선(점선)
        for (int y = 0; y < (int)worldH; ++y)
            if (y % 2 == 0)
                Renderer::Get().Submit(":", Vector2((int)(worldW * 0.5f), y), Color::Blue, 0);

        left.Draw();
        right.Draw();

        // 공
        for (const Ball& curBall : balls)
        {
            int bx = (int)std::round(curBall.GetPos().x);
            int by = (int)std::round(curBall.GetPos().y);
            Renderer::Get().Submit("O", Vector2(bx, by), Color::Yellow, 100);
        }

        // Obstacle / PowerUp
        for (const auto& obstacle : obstacles)
            obstacle->Draw();

        for (const auto& powerUp : powerUps)
            powerUp->Draw();

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

        if (roundEnded)
        {
            Renderer::Get().Submit("ROUND TIME OVER",
                Vector2((int)(worldW * 0.5f - 7), (int)(worldH * 0.5f - 3)),
                Color::Red, 100);
        }

        if (dbgPath && useAStarAI && rightAI)
        {
            AStarPositioningAI* astar = dynamic_cast<AStarPositioningAI*>(rightAI.get());
            if (astar)
            {
                const auto& path = astar->GetDebugPath();

                for (size_t i = 0; i < path.size(); ++i)
                {
                    const Int2& p = path[i];

                    if (i == 0)
                        Renderer::Get().Submit("S", Vector2(p.x, p.y), Color::Yellow, 20);
                    else if (i == path.size() - 1)
                        Renderer::Get().Submit("G", Vector2(p.x, p.y), Color::Red, 20);
                    else
                        Renderer::Get().Submit(".", Vector2(p.x, p.y), Color::Yellow, 20);
                }
            }
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
        if (dbgBallBox && !balls.empty())
        {
            /*int x0 = (int)std::round(ball.GetPos().x - 1);
            int x1 = (int)std::round(ball.GetPos().x + 1);
            int y0 = (int)std::round(ball.GetPos().y - 1);
            int y1 = (int)std::round(ball.GetPos().y + 1);*/

            const Ball& target = balls.front();

            int x0 = (int)std::round(target.GetPos().x - 1);
            int x1 = (int)std::round(target.GetPos().x + 1);
            int y0 = (int)std::round(target.GetPos().y - 1);
            int y1 = (int)std::round(target.GetPos().y + 1);

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
                else if (id == TileId::SoftWall)
                    Renderer::Get().Submit("+", Vector2(x, y), Color::Yellow, 80);
                else if (id == TileId::StickyZone)
                    Renderer::Get().Submit("~", Vector2(x, y), Color::Blue, 80);
                else if (id == TileId::WindRight)
                    Renderer::Get().Submit(">", Vector2(x, y), Color::Green, 80);
                else if (id == TileId::WindLeft)
                    Renderer::Get().Submit("<", Vector2(x, y), Color::Green, 80);
                else if (id == TileId::Bumper)
                    Renderer::Get().Submit("B", Vector2(x, y), Color::Red, 80);
            }
    }
    


    void PongWorld::ApplyZonesToBall(Ball& targetBall, float deltaTime)
    {
        {
            // 공이 존에 들어갈 때만 '한 번' 적용하면 제일 좋지만,
            // Step A에서는 간단히 "들어가 있는 동안" 약하게 적용하자.
            /*int bx = (int)std::round(ball.GetPos().x);
            int by = (int)std::round(ball.GetPos().y);*/

            int bx = (int)std::round(targetBall.GetPos().x);
            int by = (int)std::round(targetBall.GetPos().y);

            for (const auto& z : zones)
            {
                if (bx >= z.x0 && bx <= z.x1 && by >= z.y0 && by <= z.y1)
                {
                    // 너무 과하게 누적되면 속도가 폭주하니까, dt 기반으로 아주 약하게
                    Float2 v = targetBall.GetVel();
                    v.x *= std::pow(z.speedMul, deltaTime * 6.0f); // dt 보정 (대충 6은 체감 튜닝값)
                    v.y *= std::pow(z.speedMul, deltaTime * 6.0f);
                    targetBall.SetVel(v); // Ball에 setter 추가 필요(아래 참고)
                    return;
                }
            }
        }
    }

    void PongWorld::UpdateRoundTimer(float deltaTime)
    {
        if (roundEnded || matchEnded)
            return;

        roundTimeRemaining -= deltaTime;

        if (roundTimeRemaining < 0.0f)
            roundTimeRemaining = 0.0f;

        if (ShouldSpawnTimedMultiBall())
        {
            SpawnTimedMultiBall();
        }

        if (roundTimeRemaining <= 0.0f)
        {
            roundEnded = true;
        }
    }

    bool PongWorld::ShouldSpawnTimedMultiBall() const
    {
        if ((int)balls.size() >= maxBalls)
            return false;

        if (timedMultiBallStage == 0 && roundTimeRemaining <= 20.0f) return true;
        if (timedMultiBallStage == 1 && roundTimeRemaining <= 15.0f) return true;
        if (timedMultiBallStage == 2 && roundTimeRemaining <= 10.0f) return true;
        if (timedMultiBallStage == 3 && roundTimeRemaining <= 5.0f)  return true;

        return false;
    }
    void PongWorld::SpawnBall(const Float2& pos, const Float2& dir, float speed)
    {
        Ball newBall;
        newBall.Reset(pos, dir, speed);
        balls.push_back(newBall);
    }

    void PongWorld::SpawnTimedMultiBall()
    {
        if ((int)balls.size() >= maxBalls)
            return;

        if (balls.empty())
        {
            Float2 pos;
            pos.x = worldW * 0.5f;
            pos.y = worldH * 0.5f;

            Float2 dir;
            dir.x = 1.0f;
            dir.y = -0.3f;

            SpawnBall(pos, dir, 18.0f);
            ++timedMultiBallStage;
            return;
        }

        const Ball& baseBall = balls.front();
        Float2 dir = baseBall.GetVel();

        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0001f)
        {
            dir.x /= len;
            dir.y /= len;
        }
        else
        {
            dir.x = 1.0f;
            dir.y = 0.0f;
        }

        switch (timedMultiBallStage)
        {
        case 0: // 20초
            dir.x = -dir.x;
            dir.y *= 0.85f;
            break;

        case 1: // 15초
            dir.y = -dir.y;
            break;

        case 2: // 10초
            dir.x = -dir.x;
            dir.y = -dir.y * 0.9f;
            break;

        case 3: // 5초
            dir.x *= 1.05f;
            dir.y = -dir.y;
            break;
        }

        Float2 vel = baseBall.GetVel();
        float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        if (speed < 0.0001f)
            speed = 18.0f;

        SpawnBall(baseBall.GetPos(), dir, speed);
        ++timedMultiBallStage;
    }

    void PongWorld::UpdateBalls(float deltaTime)
    {
        bruteForceChecks = 0;
        spatialCandidateChecks = 0;
        actualCollisionCount = 0;
        obstacleCandidateCount = 0;
        powerUpCandidateCount = 0;

        const size_t initialCount = balls.size();
        for (size_t i = 0; i < initialCount; ++i)
        {
            UpdateBall(balls[i], deltaTime);
        }
    }

    void PongWorld::UpdateBall(Ball& curBall, float deltaTime)
    {
        curBall.Tick(deltaTime, map);
        curBall.ResolvePaddleCollision(left);
        curBall.ResolvePaddleCollision(right);
        ApplyZonesToBall(curBall, deltaTime);

        QueryAABB area;
        const Float2 p = curBall.GetPos();
        const float r = curBall.GetRadius();

        area.left = p.x - r - 2.0f;
        area.top = p.y - r - 2.0f;
        area.right = p.x + r + 2.0f;
        area.bottom = p.y + r + 2.0f;

        SpatialQueryResult result;
        spatialIndex->Query(area, result);

        obstacleCandidateCount += (int)result.obstacles.size();
        powerUpCandidateCount += (int)result.powerUps.size();

        for (Obstacle* obstacle : result.obstacles)
        {
            ++spatialCandidateChecks;
            ResolveBallVsObstacle(curBall, *obstacle);
        }

        for (PowerUp* powerUp : result.powerUps)
        {
            ++spatialCandidateChecks;
            if (CheckBallVsPowerUp(curBall, *powerUp))
            {
                ApplyPowerUp(curBall, *powerUp);
                ++actualCollisionCount;
            }
        }

        bruteForceChecks += (int)(obstacles.size() + powerUps.size());
    }

    void PongWorld::RebuildSpatialIndex()
    {
        if (!spatialIndex)
            return;

        spatialIndex->Clear();

        for (auto& obstacle : obstacles)
            spatialIndex->InsertObstacle(obstacle.get());

        for (auto& powerUp : powerUps)
            spatialIndex->InsertPowerUp(powerUp.get());
    }

    void PongWorld::ToggleSpatialIndexMode()
    {
        if (spatialMode == SpatialIndexMode::None)
        {
            spatialMode = SpatialIndexMode::Quadtree;
            spatialIndex = std::make_unique<SpatialIndex_QuadtreeAdapter>(worldW, worldH, 5, 4);
        }
        else
        {
            spatialMode = SpatialIndexMode::None;
            spatialIndex = std::make_unique<SpatialIndex_None>();
        }

        RebuildSpatialIndex();
    }

    void PongWorld::BuildStepDObjects()
    {
        obstacles.clear();
        powerUps.clear();

        const float cx = worldW * 0.5f;
        const float cy = worldH * 0.5;

        // 좌상
        obstacles.push_back(std::make_unique<Obstacle>(
            Float2(cx - 10.0f, cy - 6.0f),
            Float2(2.0f, 1.0f)
        ));


        // 우하
        obstacles.push_back(std::make_unique<Obstacle>(
            Float2(cx + 7.0f, cy + 4.0f),
            Float2(1.0f, 3.0f)
        ));

        powerUps.push_back(std::make_unique<PowerUp>(Float2{ worldW * 0.5f - 8.0f, worldH * 0.5f - 4.0f }, PowerUpType::MultiBall));
        powerUps.push_back(std::make_unique<PowerUp>(Float2{ worldW * 0.5f + 6.0f, worldH * 0.5f + 3.0f }, PowerUpType::BallSpeedUp));
    }

    bool PongWorld::IntersectsBallAABB(const Ball& ball, float left, float top, float right, float bottom) const
    {
        const Float2& p = ball.GetPos();
        const float r = ball.GetRadius();

        const float closestX = (std::max)(left, (std::min)(p.x, right));
        const float closestY = (std::max)(top, (std::min)(p.y, bottom));

        const float dx = p.x - closestX;
        const float dy = p.y - closestY;

        return (dx * dx + dy * dy) <= (r * r);
    }

    void PongWorld::ResolveBallVsObstacle(Ball& ball, const Obstacle& obstacle)
    {
        if (!IntersectsBallAABB(ball,
            obstacle.GetLeft(),
            obstacle.GetTop(),
            obstacle.GetRight(),
            obstacle.GetBottom()))
        {
            return;
        }

        const Float2 p = ball.GetPos();
        const float r = ball.GetRadius();

        const float overlapLeft = std::abs((p.x + r) - obstacle.GetLeft());
        const float overlapRight = std::abs(obstacle.GetRight() - (p.x - r));
        const float overlapTop = std::abs((p.y + r) - obstacle.GetTop());
        const float overlapBottom = std::abs(obstacle.GetBottom() - (p.y - r));

        const float minX = (std::min)(overlapLeft, overlapRight);
        const float minY = (std::min)(overlapTop, overlapBottom);

        if (minX < minY) ball.ReflectX();
        else             ball.ReflectY();

        ++actualCollisionCount;
    }

    bool PongWorld::CheckBallVsPowerUp(const Ball& ball, const PowerUp& powerUp) const
    {
        if (!powerUp.IsActive())
            return false;

        return IntersectsBallAABB(ball,
            powerUp.GetLeft(),
            powerUp.GetTop(),
            powerUp.GetRight(),
            powerUp.GetBottom());
    }
    void PongWorld::ApplyPowerUp(Ball& ball, const PowerUp& powerUp)
    {
        switch (powerUp.GetTypes())
        {
        case PowerUpType::MultiBall:
           {
            if ((int)balls.size() < maxBalls)
            {
                Float2 v = ball.GetVel();
                float len = std::sqrt(v.x * v.x + v.y * v.y);

                Float2 dir(0,0);
                if (len > 0.0001f)
                {
                    dir.x = -v.x / len;
                    dir.y = v.y / len;
                }
                else
                {
                    dir = { -1.0f, 0.2f };
                    len = 18.0f;
                }

                SpawnBall(ball.GetPos(), dir, len);
            }
            break;
        }
        case PowerUpType::BallSpeedUp:
            ball.MultiplySpeed(1.2f);
            break;

        case PowerUpType::PaddleExpand:
            break;
        }

        const_cast<PowerUp&>(powerUp).Consume();
    }
}
