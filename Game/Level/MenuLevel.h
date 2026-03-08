#pragma once

#include "Level/BaseLevel.h"
#include "Math/Color.h"
#include <vector>
#include <cstring> 

// 메뉴 아이템 구조체.
struct MenuItem
{
    using OnSelected = void (*)();

    MenuItem(const char* text, OnSelected onSelected)
        : onSelected(onSelected)
    {
        size_t length = strlen(text) + 1;
        this->text = new char[length];
        strcpy_s(this->text, length, text);
    }

    ~MenuItem()
    {
        delete[] text;
        text = nullptr;
    }

    char* text = nullptr;
    OnSelected onSelected = nullptr;
};

class MenuLevel : public BaseLevel
{
    RTTI_DECLARATIONS(MenuLevel, Level)

public:
    MenuLevel();
    ~MenuLevel();

    void Tick(float deltaTime) override;
    void Draw() override;

    static void DrawHLine(int x, int y, int w, const char* ch, Color c);
    static void DrawBrickFrame(int x, int y, int w, int h, Color c);
    static void DrawPipe(int x, int y, int height);

private:
    int currentIndex = 0;
    Color selectedColor = Color::Blue;
    Color unselectedColor = Color::White;

    std::vector<MenuItem*> items;

    float animTime = 0.0f;
    bool coinBig = false;
};