#pragma once
#include "../ActorBase.h"
class Mannequin :
    public ActorBase
{
public:
    Mannequin();
    ~Mannequin() = default;
    void Load(void) override;
    void SubInit(void) override;
};

