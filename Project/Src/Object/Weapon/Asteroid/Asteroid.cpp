#include "Asteroid.h"

#include "../../Common/Collider/BoxCollider.h"

Asteroid::Asteroid()
	:WeaponBase(Transform(), "Data/Parameter/Weapon/Asteroid/")
{
}

void Asteroid::Load(void)
{
	trans.LoadModel("Weapon/Asteroid");

	AddCollider(new BoxCollider(COLLIDER_TAG::Stage, GetParameterToVector3("Collider", "Size"), Vector3(0.0f, 0.0f, 0.0f)));

	SetDrawType(ACTOR_DRAW_TYPE::Alpha);

	SetIsDraw(false);
	SetJudgeFlg(false);

}

void Asteroid::SubInit(void)
{
	trans.scale = (SIZE);
}

void Asteroid::SubUpdate(void)
{
	//”­ŽË’†‚¾‚¯‹OÕ‚ð‹L˜^‚·‚é
	if (isFire)
	{
		trail.push_front(trans.pos);
		if (trail.size() > TRAIL_MAX) { trail.pop_back(); }
	}
	else { trail.clear(); }
}

void Asteroid::SubDraw(void)
{
	if (trail.size() < 2) { return; }

	//‰ÁŽZ‡¬
	SetWriteZBufferFlag(false);

	for (size_t i = 0; i + 1 < trail.size(); i++)
	{
		const float rate = 1.0f - static_cast<float>(i) / TRAIL_MAX;
		const int alpha = static_cast<int>(255 * rate);

		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawLine3D(trail[i].ToVECTOR(), trail[i + 1].ToVECTOR(), GetColor(120, 255, 220));
	}

	//–ß‚·
	SetWriteZBufferFlag(TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void Asteroid::SubRelease(void)
{
}
