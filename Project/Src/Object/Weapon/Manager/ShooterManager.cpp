#include "ShooterManager.h"

#include "../../../Utility/Utility.h"

#include "../../../Manager/Input/InputManager.h"

#include "../Asteroid/Asteroid.h"

ShooterManager::ShooterManager() :
	WeaponBase(Transform())
{
}

ShooterManager::ShooterManager(const Transform& ownerTrans) :
	WeaponBase(ownerTrans)
{
}

void ShooterManager::Load(void)
{
	for (int i = 0; i < div; i++)
	{
		shot[i] = new Asteroid(ownerTrans_);
		AddChildActor(shot[i]);
		shot[i]->SetIsActive(false);
	}
	state_ = STATE::CUBE;
}

void ShooterManager::SubUpdate(void)
{
	switch (state_)
	{
	case STATE::CUBE:
		CubeUpdate();
		break;

	case STATE::SPLIT:
		SpritUpdate();
		break;

	case STATE::FIRE:
		FireUpdate();
		break;
	}
	if (Input::GetIns().GetInfo(KEY_TYPE::PlayerMain).down) { ChangeState(); }
}

void ShooterManager::ChangeState(void)
{
	switch (state_)
	{
	case STATE::CUBE:

		// 状態をSPLITに変更する
		state_ = STATE::SPLIT;

		break;

	case STATE::SPLIT:

		// 状態をFIREに変更する
		state_ = STATE::FIRE;

		break;

	case STATE::FIRE:

		// 状態をCUBEに戻す
		state_ = STATE::CUBE;

		// 1つ目のアステロイドだけ有効にする
		shot[0]->SetIsActive(true);
		// 残りのアステロイドを無効にする
		for (int i = 1; i < div; ++i) { shot[i]->SetIsActive(false); }

		break;
	}
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

	// 右手の位置を基準にするため、位置を取得
	const auto frameIndex =
		MV1SearchFrame(
			ownerTrans_.model,
			"mixamorig:RightHandIndex2"
		);

	const Vector3 hand =
		MV1GetFramePosition(
			ownerTrans_.model,
			frameIndex
		);

	// オーナーの右方向を取得
	const Vector3 right =
		ownerTrans_.VTrans(
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
			ownerTrans_.model,
			"mixamorig:RightHandIndex2"
		);

	Vector3 handPos =
		MV1GetFramePosition(
			ownerTrans_.model,
			frameIndex
		);

	Vector3 scale = 0.3f;

	// オーナーの前方向
	const Vector3 forward =
		ownerTrans_.VTrans(
			Vector3(0.0f, 0.0f, 1.0f)
		);

	// オーナーの右方向
	const Vector3 right =
		ownerTrans_.VTrans(
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
	const float speed = 100.0f;

	Vector3 scale = 0.3f;

	for (const auto& s : shot)
	{
		Asteroid* asteroid = s;

		auto sp = GetRand(speed);
		if (sp < 50) { sp = 70; }

		Vector3 centerPos =
			asteroid->GetTrans().pos +
			asteroid->GetDir() * sp;

		asteroid->SetTrans(centerPos, scale);
	}
}