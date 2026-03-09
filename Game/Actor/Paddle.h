#pragma once
#include "Common/Types.h"

class Paddle
{
public :
	inline void Reset(const Float2& p) { pos = p; velY = 0.0f; moveInput = 0.0f; }
	
	inline void SetMoveInput(float v) { moveInput = v; }
	void Tick(float deltaTime, float worldH);
	void Draw();

	inline const Float2& GetPos() const { return pos; }

	inline float GetHalfW() const { return halfW; }
	inline float GetHalfH() const { return halfH; }
	inline float GetVelY() const { return velY; }

private :
	Float2 pos{ 0,0 };
	float moveInput = 0.0f;

	float speed = 40.0f;
	float velY = 0.0f;

	float halfW = 0.2f;
	float halfH = 1.0f;
};

