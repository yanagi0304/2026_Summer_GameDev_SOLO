#include "ShooterManager.h"
#include "../Asteroid/Asteroid.h"
#include "../../../Manager/Input/KeyManager.h"
#include "../../../Utility/Utility.h"

ShooterManager::ShooterManager()
{
}

ShooterManager::ShooterManager(const Transform& ownerTrans)
{
	for (int i = 0; i < 8; i++)
	{
		shot[i] = std::make_unique<Asteroid>(ownerTrans);
		shot[i]->Load();
	}
	state_ = STATE::CUBE;
	ownerTrans_ = ownerTrans;
}

void ShooterManager::WeaponLoad(void)
{

}

void ShooterManager::WeaponInit(void)
{
}

void ShooterManager::WeaponUpdate(void)
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
	ChangeState();
	
}

void ShooterManager::WeaponDraw(void)
{
	switch (state_)
	{
	case STATE::CUBE:
		CubeDraw();
		break;

	case STATE::SPLIT:
		SpritDraw();
		break;

	case STATE::FIRE:
		FireDraw();
		break;
	}

}

void ShooterManager::WeaponAlphaDraw(void)
{

}

void ShooterManager::WeaponUiDraw(void)
{
}

void ShooterManager::WeaponRelease(void)
{
}

void ShooterManager::ChangeState(void)
{
	if (KeyManager::GetIns().GetInfo(KEY_TYPE::PLAYER_MAIN).down)
	{
		switch (state_)
		{
		case STATE::CUBE:
			state_ = STATE::SPLIT;
			break;

		case STATE::SPLIT:
			state_ = STATE::FIRE;
			break;

		case STATE::FIRE:
			state_ = STATE::CUBE;
			break;
		}
	}
}

void ShooterManager::CubeUpdate()
{
	shot[0]->Update();
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
	Vector3 handPos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);

	for (const auto& s : shot)
	{
		Asteroid* asteroid = static_cast<Asteroid*>(s.get());
		asteroid->SetTrans(handPos, Vector3(0.6f));
	}
}

void ShooterManager::SpritUpdate()
{
	// 右手の位置を基準にするため、位置を取得
	auto frameIndex = MV1SearchFrame(ownerTrans_->get().model, "mixamorig:RightHandIndex2");
	Vector3 handPos = MV1GetFramePosition(ownerTrans_->get().model, frameIndex);

	Vector3 scale = 0.3f;

	float angle = ownerTrans_->get().angle.y;

	Vector3 forward(
		sinf(angle),
		0.0f,
		cosf(angle));

	Vector3 right(
		cosf(angle),
		0.0f,
		-sinf(angle));

	Vector3 centerPos =
		handPos -
		right * 80.0f +
		Vector3(0.0f, 60.0f, 0.0f);


	//アステロイド分割の形
	std::vector<Vector3> offsets =
	{
		{-1,  1, 0},
		{ 0,  1, 0},
		{ 1,  1, 0},

		{-1,  0, 0},
		{ 1,  0, 0},

		{-1, -1, 0},
		{ 0, -1, 0},
		{ 1, -1, 0},
	};

	for (int i = 0; i < offsets.size(); ++i)
	{
		if (shot[i] == nullptr) { continue; }

		Asteroid* asteroid = static_cast<Asteroid*>(shot[i].get());
		Vector3 offset =
			right * (offsets[i].x * 25.0f) +
			Vector3(0.0f, offsets[i].y * 25.0f, 0.0f);

		asteroid->SetTrans(centerPos + offset, scale);
		asteroid->SetDir(forward);
	}



}

void ShooterManager::FireUpdate()
{

	const float speed = 30.0f;

	Vector3 scale = 0.3f;

	for (const auto& s : shot)
	{
		Asteroid* asteroid = static_cast<Asteroid*>(s.get());

		auto sp = GetRand(speed);
		if (sp < 20) { sp = 20; }

		Vector3 centerPos =
			asteroid->GetTrans().pos -
			asteroid->GetDir() * sp;

		asteroid->SetTrans(centerPos, scale);
	}
}

void ShooterManager::CubeDraw()
{
	shot[0]->Draw();
}

void ShooterManager::SpritDraw()
{
	for (const auto& s : shot)
	{
		s->Draw();
	}
}

void ShooterManager::FireDraw()
{
	for (const auto& s : shot)
	{
		s->Draw();
	}
}
