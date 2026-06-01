#pragma once
#include "../ActorBase.h"
class Stage :
    public ActorBase
{
public:

    Stage();
    ~Stage() = default;
    void Load(void) override;
    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
	void Release(void) override;

private:


};

