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

	Vector3 sclae = 0.3f;

	for (int i = 0; i < 8; ++i)
	{
		if (shot[i] == nullptr) { continue; }

		Asteroid* asteroid = static_cast<Asteroid*>(shot[i].get());
		Vector3 targetPos;
		targetPos.x = handPos.x - 80.0f - (sinf(i * 2.3f) * 30.0f);
		targetPos.y = handPos.y + 60.0f + (cosf(i * 1.7f) * 25.0f);
		targetPos.z = handPos.z + (sinf(i * 3.1f) * 30.0f);

		
		asteroid->SetTrans(targetPos, sclae);
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
