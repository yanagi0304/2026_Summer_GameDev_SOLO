#pragma once
#include "../WeaponBase.h"

#include <vector>
#include <memory>

#include "../../Common/AnimationController/CharacterAnimeTypeDefine.h"

class Asteroid;

enum class CUBE_STATE
{
	NONE,
	CUBE,
	SPLIT,
	FIRE,
};

class ShooterManager
	: public WeaponBase
{
public:

	ShooterManager(
		const Transform& ownerTrans,
	
		std::function<float(void)> animeRatio,
		std::function<void(void)> playAnime
	);

	~ShooterManager() = default;

	void Load(void)override;

	int GetIdleAnimeID(void) const override;
	int GetAttackAnimeID(void) const override { return CHARACTER_ANIME::SLASH_A1; }

	//ステートのセッター
	void SetState(const CUBE_STATE cubeState) { state = cubeState; }

	//選択されたとき
	void OnEquip(void)override;

	void OnUnequip(void)override;

	static constexpr int div = 27;

	static constexpr int FIRE_INTERVAL = 3;

private:

	void SubUpdate(void) override;

	void SubDraw(void)override;

	//ステート切り替え
	void ChangeState(void);

	//各状態の更新
	void CubeUpdate();
	void SpritUpdate();
	void FireUpdate();

	//発射速度、向きなどを発射前に決める
	void GetFire(void);

	//エイム入力
	void AimInput(void);

	//強制ズームアウト
	void ZoomOut(void);

private:
	int divideNum_;

	CUBE_STATE state;
	Asteroid* shot[div];

   int fireIndex_ = 0;
   int fireTimer_ = 0;

   // 画面中央のX座標
   int screenCenterX;
   // 画面中央のY座標
   int screenCenterY;

   //発射角度
   Vector3 dir;

#pragma region 受け取る関数

   // アニメーションの再生割合を取得する関数のポインタ
   std::function<float(void)> animeRatio;

   //状態切り替え時のアニメーションを取得する関数ポインタ
   std::function<void(void)> playAnime;

#pragma endregion

   // 右手の位置を取得する(GetHandPos:ゲット ハンド ポス)
   Vector3 GetHandPos(void) const;
   // アステロイド群の中心位置を取得する(GetCenterPos:ゲット センター ポス)
   Vector3 GetCenterPos(void) const;
};

