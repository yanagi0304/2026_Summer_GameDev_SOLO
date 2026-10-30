#include "ShoulderCamera.h"

#include <cmath>
#include <algorithm>

#include "../../../Utility/Utility.h"

#include "../../Input/InputManager.h"

ShoulderCamera::ShoulderCamera(const Vector3* targetPos, float fov)
    :CameraBase(Vector3(), Vector3(), fov),

    targetPos(targetPos),
    pivotPos(Vector3()),

    yaw(0.0f),
    pitch(Deg2Rad(10.0f)),

    pitchMin(Deg2Rad(-35.0f)),
    pitchMax(Deg2Rad(70.0f)),

    rotPower(Deg2Rad(3.0f)),

    isAiming(false),
    aimRate(0.0f),
    aimBlendSpeed(0.2f),

    pivotHeight(250.0f),

    normalDistance(420.0f),
    aimDistance(120.0f),

    normalShoulder(100.0f),
    aimShoulder(45.0f),

    normalFov(fov),
    aimFov(fov * 0.85f)
{
    //視点回転で画面恥に当たらないように、マウスを中心に固定
    Input::GetIns().SetMouseFixed(true);
}

void ShoulderCamera::Init(void)
{
    //追従対象がいなければ何もしない
    if (targetPos == nullptr) { return; }

    //基準点を補間せずに合わせる
    pivotPos = *targetPos + Vector3(0.0f, pivotHeight, 0.0f);

    //エイム補間率も現在の状態に合わせる
    aimRate = isAiming ? 1.0f : 0.0f;

    //カメラへ反映
    UpdateTransform();

}

Vector3 ShoulderCamera::GetForwardXZ(void) const
{
    return Vector3(std::sin(yaw), 0.0f, std::cos(yaw));
}

Vector3 ShoulderCamera::GetRightXZ(void) const
{
    return Vector3(std::cos(yaw), 0.0f, -std::sin(yaw));
}

Vector3 ShoulderCamera::CalcForward(void) const
{
    const float cosP = std::cos(pitch);
    return Vector3(std::sin(yaw) * cosP,
        -std::sin(pitch),
        std::cos(yaw) * cosP
    );
}

void ShoulderCamera::UpdateTransform(void)
{
    //エイム補間率に応じた距離と肩越しオフセット
    const float distance = Lerp(normalDistance, aimDistance, aimRate);
    const float shoulder = Lerp(normalShoulder, aimShoulder, aimRate);

    //基準点から右へずらし、前方向の後ろへ下げた位置にカメラを置く
    //注視点を追従対象にせず前方向と平行に向けるため、追従対象は画面の左寄りに映る
    pos = pivotPos + GetRightXZ() * shoulder - CalcForward() * distance;

    // 回転と視野角を反映する
    SetAngle(Vector3(pitch, yaw, 0.0f));
    SetFov(Lerp(normalFov, aimFov, aimRate));
}

void ShoulderCamera::NormalUpdate(void)
{
    //追従対象がなければ何もしない
    if (targetPos == nullptr) { return;}

    //回転入力を取得して角度に反映する
    Vector3 input;
    if (RotationInput(input)) {

        const float scale = rotPower * GetTimeScale();

        yaw += input.y * scale;
        pitch += input.x * scale;

        //ピッチを制限する
        pitch = std::clamp(pitch, pitchMin, pitchMax);
    }

    //エイム補間率を目標率へ近づける
    const float aimGoal = isAiming ? 1.0f : 0.0f;
    aimRate += (aimGoal - aimRate) * aimBlendSpeed * GetTimeScale();

    // 基準点を追従対象へ補間しながら近づける
    SmoothCameraMove(pivotPos, *targetPos + Vector3(0.0f, pivotHeight, 0.0f));

    //カメラへ反映
    UpdateTransform();

}

void ShoulderCamera::SubRelease(void)
{
    Input::GetIns().SetMouseFixed(false);
}
