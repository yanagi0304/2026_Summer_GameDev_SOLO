#include "MannequinManager.h"

#include "Mannequin.h"

MannequinManager::MannequinManager()
	:ActorBase("Data/Parameter/Mannequin/")
{
}

void MannequinManager::Load(void)
{
	// マネキン生成
	for (int i = 1; ; i++) {
		// その回のパラメーター名
		std::string parameterPos = "Pos" + std::to_string(i);

		// そのパラメータが存在するかどうか
		if (!IsParameterExist("Init", parameterPos)) { break; }

		// その回のパラメーター名
		std::string parameterDynamicFlg = "DynamicFlg" + std::to_string(i);
		// そのパラメータが存在するかどうか
		if (!IsParameterExist("Init", parameterDynamicFlg)) { break; }

		// 存在するならば、その座標にその敵を生成
		AddChildActor(new Mannequin(GetParameterToVector3("Init", parameterPos), GetParameterToBool("Init", parameterDynamicFlg)));
	}
}
