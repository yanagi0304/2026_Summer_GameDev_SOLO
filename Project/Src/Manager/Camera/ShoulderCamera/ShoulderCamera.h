#pragma once
#include "../CameraBase.h"

// 通常時は追従対象を画面左よりにして、エイム時は対象に寄る
class ShoulderCamera :
    public CameraBase
{
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="targetPos">追従対象</param>
    /// <param name="fov">通常時の視野角</param>
    ShoulderCamera(const Vector3* targetPos, float fov = 60.0f * (DX_PI_F / 180.0f));
    ~ShoulderCamera()override = default;

    //初期化
    void Init(void)override;

    //追従対象の変更
    void TargetChange(const Vector3* targetPos) {
        if (targetPos == nullptr) { return; }
        this->targetPos = targetPos;
    }

    //エイム状態を設定する
    void SetAiming(bool flg) { isAiming = flg; }

    //エイム中か
    bool IsAiming(void)const { return isAiming; }

    //エイム保管率を取得
    float GetAimRate(void) const { return aimRate; }

    // 水平方向の前ベクトルを取得。プレイヤー移動の基準に使う
    Vector3 GetForwardXZ(void) const;

    // 水平方向の右ベクトルを取得
    Vector3 GetRightXZ(void) const;

private:

    //追従対象
    const Vector3* targetPos;

    //追従中の基準点
    Vector3 pivotPos;

#pragma region 角度関係

    //水平回転角
    float yaw;

    //垂直回転角
    float pitch;

    //ピッチ下限
    float pitchMin;

    //ピッチ上限
    float pitchMax;

    //回転入力の倍率
    float rotPower;

#pragma endregion

    //エイム中か
    bool isAiming;

    //エイム保管率
    float aimRate;

    //エイム保管の速さ
    float aimBlendSpeed;

    //基準点の高さ
    float pivotHeight;

    //通常時の追従対象からの距離
    float normalDistance;

    //エイム時の追従対象からの距離
    float aimDistance;

    //通常時の肩越しオフセット
    float normalShoulder;

    //エイム時の肩越しオフセット
    float aimShoulder;

    //通常時の視野角
    float normalFov;

    //エイム時の視野角
    float aimFov;

    //現在の角度から前方ベクトル取得
    Vector3 CalcForward(void)const;

    //現在の状態からカメラの座標・回転・視野角を反映する
    void UpdateTransform(void);

    //更新
    void NormalUpdate(void)override;

    //終了
    void SubRelease(void)override;

};

