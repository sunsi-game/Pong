#include "MenuLevel.h"
#include "Game/Game.h"
#include "Core/Input.h"
#include "Util/Util.h"
#include "Game/GameManager.h"
#include "Render/Renderer.h"
#include <iostream>

MenuLevel::MenuLevel()
{

	// 메뉴 아이템 생성.
	items.emplace_back(new MenuItem(
		"Start",
		[]()
		{
			// 메뉴 토글 함수 호출.
			//Game::Get().StartGame();
			GameManager::Get().StartNewGame(3);
		}
	));

	items.emplace_back(new MenuItem(
		"Quit",
		[]()
		{
			// 게임 종료.
			Game::Get().QuitEngine();
		}
	));
}

MenuLevel::~MenuLevel()
{
	for (MenuItem*& item : items)
	{
		delete item;
		item = nullptr;
	}

	// 배열 초기화.
	items.clear();
}

void MenuLevel::Tick(float deltaTime)
{
	super::Tick(deltaTime);

	Renderer::Get().SetCameraOffset(Vector2::Zero);

	animTime += deltaTime;
	if (animTime >= 0.25f) // 0.25초마다 토글.
	{
		animTime = 0.0f;
		coinBig = !coinBig;
	}

	// 입력처리.
	// 배열 길이.
	static int length = static_cast<int>(items.size());

	if (Input::Get().GetKeyDown(VK_UP))
	{
		// 인덱스 돌리기.
		// 개인적으로 중요한 정보.
		currentIndex = (currentIndex - 1 + length) % length;
	}

	if (Input::Get().GetKeyDown(VK_DOWN))
	{
		// 인덱스 돌리기 (나머지 연산은 배열의 길이로 해주면 무한으로 돌릴 수 있다).
		currentIndex = (currentIndex + 1) % length;
	}

	if (Input::Get().GetKeyDown(VK_RETURN))
	{
		items[currentIndex]->onSelected();
	}

	if (Input::Get().GetKeyDown(VK_ESCAPE))
	{
		// 메뉴 토글.
		//Game::Get().ToggleMenu();

		// 인덱스 초기화.
		currentIndex = 0;
	}
}

void MenuLevel::Draw()
{
	const int originX = 8;
	const int originY = 3;
	const int frameW = 44;
	const int frameH = 20;

	// 외곽 프레임
	DrawBrickFrame(originX, originY, frameW, frameH, Color::White);

	// 중앙 점선
	for (int y = originY + 1; y < originY + frameH - 1; y += 2)
	{
		Renderer::Get().Submit("|", Vector2(originX + frameW / 2, y), Color::Blue);
	}

	// 좌우 패들 장식
	for (int i = 0; i < 4; ++i)
	{
		Renderer::Get().Submit("|", Vector2(originX + 3, originY + 6 + i), Color::White);
		Renderer::Get().Submit("|", Vector2(originX + frameW - 4, originY + 10 + i), Color::White);
	}

	// 공 장식 (깜빡이게)
	const char* ball = coinBig ? "O" : "o";
	Renderer::Get().Submit(ball, Vector2(originX + frameW / 2 - 1, originY + 6), Color::Yellow);

	// 타이틀
	Renderer::Get().Submit("P O N G", Vector2(originX + 16, originY + 2), Color::White);

	// 부제 느낌
	Renderer::Get().Submit("CLASSIC ARCADE MODE", Vector2(originX + 11, originY + 4), Color::White);

	// 메뉴 위치
	int menuX = originX + 16;
	int menuY = originY + 9;

	for (int i = 0; i < static_cast<int>(items.size()); ++i)
	{
		const bool selected = (i == currentIndex);
		Color textColor = selected ? Color::Yellow : Color::White;

		if (selected)
		{
			Renderer::Get().Submit(">", Vector2(menuX - 3, menuY + i * 2), Color::Yellow);
			Renderer::Get().Submit("<", Vector2(menuX + 8, menuY + i * 2), Color::Yellow);
		}

		Renderer::Get().Submit(items[i]->text, Vector2(menuX, menuY + i * 2), textColor);
	}
}

void MenuLevel::DrawHLine(int x, int y, int w, const char* ch, Color c)
{
	for (int i = 0; i < w; ++i)
		Renderer::Get().Submit(ch, Vector2(x + i, y), c);
}

void MenuLevel::DrawBrickFrame(int x, int y, int w, int h, Color c)
{
	
	DrawHLine(x, y, w, "-", c);
	DrawHLine(x, y + h - 1, w, "-", c);

	for (int j = 1; j < h - 1; ++j)
	{
		Renderer::Get().Submit("|", Vector2(x, y + j), c);
		Renderer::Get().Submit("|", Vector2(x + w - 1, y + j), c);
	}

}
