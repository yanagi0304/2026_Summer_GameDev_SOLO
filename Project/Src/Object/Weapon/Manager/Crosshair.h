#pragma once

// 画面中央の照準(Crosshair:クロスヘア=十字線)
class Crosshair
{
public:

	// コンストラクタ(画面中央の座標を決める)
	Crosshair(void);

	~Crosshair(void) = default;

	// 描画する
	void Draw(void) const;

private:

	// 十字の腕の長さ(中心から端まで)
	static constexpr int ARM_LENGTH = 20;

	// 縁取り(黒)の太さ
	static constexpr int OUTLINE_THICKNESS = 4;

	// 本体(白)の太さ
	static constexpr int LINE_THICKNESS = 2;

	// 画面中央のX座標
	int centerX_;

	// 画面中央のY座標
	int centerY_;

};
