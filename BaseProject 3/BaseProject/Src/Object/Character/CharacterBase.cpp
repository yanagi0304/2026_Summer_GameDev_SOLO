
#include"CharacterBase.h"

CharacterBase::CharacterBase(
	short HP_MAX,
	short ATTACK_POWER,
	short DEFENSE_POWER,
	short SPEED_POWER
) :
	characterStats(HP_MAX, ATTACK_POWER, DEFENSE_POWER, SPEED_POWER),

	ActorBase(),

	state(0),
	stateMap(),

	DEFAULT_COLOR(),

	anime(nullptr),

	inviCounter(0),
	isInviEffect(false),

	operatorSenderId(MSG_SENDER_ID::None),
	isOwnOperator(false),

	animeType_(AnimationController::CHARACTER_ANIME::T)
{
}

CharacterBase::CharacterBase(
	short HP_MAX,
	short ATTACK_POWER,
	short DEFENSE_POWER,
	short SPEED_POWER,
	const std::string& parameterPath
) :
	ActorBase(parameterPath),

	state(0),
	stateMap(),

	DEFAULT_COLOR(),

	anime(nullptr),

	inviCounter(0),
	isInviEffect(false),

	characterStats(HP_MAX, ATTACK_POWER, DEFENSE_POWER, SPEED_POWER),

	operatorSenderId(MSG_SENDER_ID::None),
	isOwnOperator(false),

	isDeath(false),

	animeType_(AnimationController::CHARACTER_ANIME::T)
{
}
	
void CharacterBase::Load(void)
{
	CharacterLoad();
}

void CharacterBase::SubInit(void)
{
	// モデルのカラーの初期化
 	SetInviEffectFlg();

	// キャラクター固有の初期化
	CharactorInit();
}

void CharacterBase::SubUpdate(void)
{
	if (isOwnOperator) {
		// 無敵カウンターの更新
		Invi();

		// キャラクター固有の更新
		CharactorUpdate();

		// ステートの更新
		if (stateMap.size() > 0) {
			stateMap.at(state)->OtherStateConditionsUpdate();
			stateMap.at(state)->Update();
			for (std::pair<const int, CharacterStateBase*>& s : stateMap) { s.second->AlwaysUpdate(); }
		}
	}
	else {
		// 操作権を持たないキャラクターが呼ぶ更新処理

	}

	// アニメーション更新
	if (anime) { anime->Update(); }
}

void CharacterBase::SubDraw(void)
{
	// キャラクター固有の描画
	CharactorDraw();
}

void CharacterBase::SubAlphaDraw(void)
{
	// キャラクター固有の描画
	CharactorAlphaDraw();
}

void CharacterBase::UiDraw(void)
{
	CharacterUiDraw();
}

void CharacterBase::SubRelease(void)
{
	// キャラクター固有の解放
	CharactorRelease();

	// ステート管理用マップの解放
	for (auto& s : stateMap) {
		if (s.second) { delete s.second; }
	}
	stateMap.clear();

	// デフォルトカラー情報の解放
	if (!DEFAULT_COLOR.empty()) {
		DEFAULT_COLOR.clear();
	}

	// アニメーションコントローラーの解放（使われていたら）
	if (anime) {
		anime->Release();
		delete anime;
		anime = nullptr;
	}
}

void CharacterBase::SetInviCounter(unsigned char counter)
{
	inviCounter = (counter < 0) ? 0 : (counter > 255) ? 255 : counter;

	// 0 以下の代入であればついでに（バグ防止のために）
	if (counter <= 0) {
		for (int i = 0; i < DEFAULT_COLOR.size(); i++) {
			MV1SetMaterialEmiColor(trans.model, i, DEFAULT_COLOR[i]);
		}
	}
}

void CharacterBase::Invi(void)
{
	if (inviCounter > 0) { inviCounter--; }
	else { inviCounter = 0; }

	// ダメージ演出
	if (!isInviEffect) { return; }

	if (inviCounter > 1) {
		if (inviCounter / 10 % 2 == 0) {
			for (int i = 0; i < DEFAULT_COLOR.size(); i++) {
				MV1SetMaterialEmiColor(trans.model, i, DEFAULT_COLOR[i]);
			}
		}
		else {
			for (int i = 0; i < DEFAULT_COLOR.size(); i++) {
				COLOR_F emi = DEFAULT_COLOR[i];
				emi.r = (std::min)(DEFAULT_COLOR[i].r + 0.6f, 1.0f);
				MV1SetMaterialEmiColor(trans.model, i, emi);
			}
		}

	}
	else if (inviCounter == 1) {
		for (int i = 0; i < DEFAULT_COLOR.size(); i++) {
			MV1SetMaterialEmiColor(trans.model, i, DEFAULT_COLOR[i]);
		}
	}
}

void CharacterBase::AddInFbxAnimation(int inFbxMaxIndex, float speed)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed, index);
	}
}

void CharacterBase::AddInFbxAnimation(int inFbxMaxIndex, const float* speed)
{
	for (int index = 0; index < inFbxMaxIndex; index++) {
		anime->AddInFbx(index, speed[index], index);
	}
}

void CharacterBase::AddAnimation(int index, float speed, const char* filePath)
{
	anime->Add(index, speed, filePath);
}