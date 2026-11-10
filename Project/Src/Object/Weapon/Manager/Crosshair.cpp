#include "Crosshair.h"

#include "../../../pch.h"

#include "../../../Application/Application.h"

Crosshair::Crosshair(void) :
	centerX_(Application::SCREEN_SIZE_X_HALF),
	centerY_(Application::SCREEN_SIZE_Y_HALF)
{
}

void Crosshair::Draw(void) const
{
	// 明るい背景でも暗い背景でも見えるよう、黒の縁取りを先に描く
	DrawLine(centerX_ - ARM_LENGTH - 1, centerY_, centerX_ + ARM_LENGTH + 1, centerY_, 0x000000, OUTLINE_THICKNESS);
	DrawLine(centerX_, centerY_ - ARM_LENGTH - 1, centerX_, centerY_ + ARM_LENGTH + 1, 0x000000, OUTLINE_THICKNESS);

	// 白い十字を重ねて描く
	DrawLine(centerX_ - ARM_LENGTH, centerY_, centerX_ + ARM_LENGTH, centerY_, 0xffffff, LINE_THICKNESS);
	DrawLine(centerX_, centerY_ - ARM_LENGTH, centerX_, centerY_ + ARM_LENGTH, 0xffffff, LINE_THICKNESS);
}
