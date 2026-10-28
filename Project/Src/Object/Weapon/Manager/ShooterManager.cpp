#include "ShooterManager.h"

#include "../../../Utility/Utility.h"

#include "../../../Application/Application.h"

#include "../../../Manager/Input/InputManager.h"

#include "../../../Manager/Camera/CurrentCamera.h"

#include "../Asteroid/Asteroid.h"

ShooterManager::ShooterManager(
	const Transform& ownerTrans,

	std::function<float(void)> animeRate,
	std::function<void(void)> playAnime
) :
	WeaponBase(ownerTrans),
	animeRatio(animeRate),
	playAnime(playAnime),
	screenCenterX(Application::SCREEN_SIZE_X_HALF),
	screenCenterY(Application::SCREEN_SIZE_Y_HALF),
	dir()
{
	//SetDrawType(ACTOR_DRAW_TYPE::Alpha);
}

void ShooterManager::Load(void)
{
	for (int i = 0; i < div; i++)
	{
		shot[i] = new Asteroid(dir);
		AddChildActor(shot[i]);
		shot[i]->SetIsActive(false);
	}
	state = CUBE_STATE::NONE;

	SetJudgeFlg(true);
	SetDynamicFlg(true);
	SetPushFlg(false);

}

int ShooterManager::GetIdleAnimeID(void) const
{
	switch (state)
	{
	case CUBE_STATE::NONE:
		return CHARACTER_ANIME::ASTEROID_IDLE_R;
		break;
	case CUBE_STATE::CUBE:
		return CHARACTER_ANIME::ASTEROID_CREATE_R;
		break;
	case CUBE_STATE::SPLIT:
		return CHARACTER_ANIME::ASTEROID_CREATE_R;
		break;
	case CUBE_STATE::FIRE:
		return CHARACTER_ANIME::ASTEROID_IDLE_R;
		break;
	}
}

void ShooterManager::OnEquip(void)
{

	state = state = CUBE_STATE::NONE;
	for (auto* s : shot)
	{
		s->SetIsActive(false);
	}
}

void ShooterManager::OnUnequip(void)
{
	SetIsDraw(true);
	for (auto* s : shot)
	{
		s->SetIsActive(false);
	}
}

void ShooterManager::SubUpdate(void)
{
	switch (state)
	{
	case CUBE_STATE::CUBE:
		CubeUpdate();
		break;

	case CUBE_STATE::SPLIT:
		SpritUpdate();
		break;

	case CUBE_STATE::FIRE:
		FireUpdate();
		break;
	}
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) { ChangeState(); }
}

void ShooterManager::ChangeState(void)
{
	switch (state)
	{
	case CUBE_STATE::NONE:

		// 状態をSPLITに変更する
		state = CUBE_STATE::CUBE;
		// 1つ目のアステロイドだけ有効にする
		shot[0]->SetIsActive(true);
	
		break;
	case CUBE_STATE::CUBE:

		// 状態をSPLITに変更する
		state = CUBE_STATE::SPLIT;

		break;

	case CUBE_STATE::SPLIT:

		// 状態をFIREに変更する
		state = CUBE_STATE::FIRE;
		for (auto* s : shot) { s->SetFire(true); 
		}
		GetFire();

		break;

	case CUBE_STATE::FIRE:

		// 状態をCUBEに戻す
		state = CUBE_STATE::CUBE;
		for (auto* s : shot) { s->SetFire(false); s->ClearTrail(); }
		// 1つ目のアステロイドだけ有効にする
		shot[0]->SetIsActive(true);
		// 残りのアステロイドを無効にする
		for (int i = 1; i < div; ++i) { shot[i]->SetIsActive(false); }
		break;
	}

	playAnime();
}

void ShooterManager::CubeUpdate()
{
	//shot[0]->Update();

	//auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
	//Vector3 handPos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);

	//for (const auto& s : shot)
	//{
	//	Asteroid* asteroid = static_cast<Asteroid*>(s.get());
	//	asteroid->SetTrans(handPos, Vector3(asteroid->GetSize()));
	//}

	// アステロイドの取得
	Asteroid* asteroid = shot[0];

	asteroid->SetIsActive(true);

	// 右手の位置を基準にするため、位置を取得
	const auto frameIndex =
		MV1SearchFrame(
			ownerTrans.model,
			"mixamorig:RightHandIndex2"
		);

	const Vector3 hand =
		MV1GetFramePosition(
			ownerTrans.model,
			frameIndex
		);

	// オーナーの右方向を取得
	const Vector3 right =
		ownerTrans.VTrans(
			Vector3(1.0f, 0.0f, 0.0f)
		);

	asteroid->SetTrans(
		hand +
		right * 80.0f +
		Vector3(0.0f, 60.0f, 0.0f),
		asteroid->SIZE
	);
}

void ShooterManager::SpritUpdate()
{
	// 右手の位置を基準にするため、位置を取得
	auto frameIndex =
		MV1SearchFrame(
			ownerTrans.model,
			"mixamorig:RightHandIndex2"
		);

	Vector3 handPos =
		MV1GetFramePosition(
			ownerTrans.model,
			frameIndex
		);

	Vector3 scale = 0.3f;

	// オーナーの前方向
	const Vector3 forward =
		ownerTrans.VTrans(
			Vector3(0.0f, 0.0f, 1.0f)
		);

	// オーナーの右方向
	const Vector3 right =
		ownerTrans.VTrans(
			Vector3(1.0f, 0.0f, 0.0f)
		);

	Vector3 centerPos =
		handPos +
		right * 80.0f +
		Vector3(0.0f, 60.0f, 0.0f);

	// アステロイド分割の形
	std::vector<Vector3> offsets;

	static constexpr int SIZE = 3;
	static constexpr float SPACE = 50.0f;

	for (int z = 0; z < SIZE; z++)
	{
		for (int y = 0; y < SIZE; y++)
		{
			for (int x = 0; x < SIZE; x++)
			{
				Vector3 offset(
					(x - (SIZE - 1) * 0.5f),
					((SIZE - 1) * 0.5f - y),
					(z - (SIZE - 1) * 0.5f)
				);

				offsets.emplace_back(offset);
			}
		}
	}

	// 必要なアステロイドの数だけ表示する
	for (int i = 0; i < offsets.size(); ++i)
	{
		if (shot[i] == nullptr) { continue; }

		Asteroid* asteroid = shot[i];

		Vector3 offset =
			right * (offsets[i].x * 25.0f) +
			Vector3(0.0f, offsets[i].y * 25.0f, 0.0f) +
			forward * (offsets[i].z * 25.0f);

		asteroid->SetTrans(
			centerPos + offset,
			scale
		);

		asteroid->SetDir(forward);

		asteroid->SetIsActive(true);
	}

	// 余ったアステロイドを非表示にする
	for (int i = static_cast<int>(offsets.size()); i < div; ++i)
	{
		if (shot[i] == nullptr) { continue; }

		Asteroid* asteroid = shot[i];

		asteroid->SetIsActive(false);
	}

}

void ShooterManager::FireUpdate()
{


}

void ShooterManager::GetFire()
{

	// カメラの角度(angle:アングル=角度)
	Vector3 camAngle = CurrentCamera::Get().GetAngle();

	// 上下の回転(pitch:ピッチ)の影響を含めた前方向ベクトル
	// sinf:サイン・エフ(正弦) cosf:コサイン・エフ(余弦)
	dir = VGet(
		sinf(camAngle.y) * cosf(camAngle.x),  // 横方向(X)
		0.0f,                    // 上下方向(Y)
		cosf(camAngle.y) * cosf(camAngle.x)   // 奥方向(Z)
	);

}
