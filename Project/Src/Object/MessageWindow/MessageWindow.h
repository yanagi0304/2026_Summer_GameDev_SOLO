#pragma once
#include "../Common/ActorBase/ActorBase.h"

class MessageWindow : public ActorBase
{
public:

	MessageWindow(void);
	~MessageWindow(void)override = default;

	//”h¶æ’Ç‰Á‰Šú‰»
	void SubInit(void)override;
	// ”h¶æ’Ç‰ÁXV
	void SubUpdate(void)override;
	// ”h¶æ’Ç‰Á•`‰æ
	void SubDraw(void)override;
	// ”h¶æ’Ç‰Á‰ğ•ú
	void SubRelease(void)override;

private:


};

