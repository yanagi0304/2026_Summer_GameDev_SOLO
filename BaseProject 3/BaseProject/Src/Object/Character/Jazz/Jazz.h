#pragma once
#include "../CharacterBase.h"
class Jazz :
	public CharacterBase
{
public:
	Jazz();
	~Jazz() override = default;

private:

	void CharacterLoad(void)override;
	void CharactorInit(void)override;
	void CharactorUpdate(void)override;
	void CharactorDraw(void)override;
	void CharactorAlphaDraw(void)override;
	void CharacterUiDraw(void)override;
	void CharactorRelease(void)override;
	void SubUpdate(void)override;
};

