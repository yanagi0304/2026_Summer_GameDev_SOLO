#pragma once
#include "../WeaponBase.h"
class Senku :
    public WeaponBase
{
public:

    Senku(const Transform& ownerTrans, const DecreaseTrionFunc& decreaseTrion = nullptr);
    ~Senku() override = default;
    void Load(void) override;
    int GetIdleAnimeID(void) const override { return 0; }
	int GetAttackAnimeID(void) const override { return 0; }

private:

    void SubInit(void) override;
    void SubUpdate(void) override;
    void SubDraw(void) override;
    void SubRelease(void) override;

};

