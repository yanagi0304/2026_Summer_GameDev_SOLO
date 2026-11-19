#pragma once

#include "../../../Manager/TimeScale/TimeScale.h"

#include "../PhysicsBody/PhysicsBody.h"

// 移動操作の共通基底。PhysicsBodyの所有権は持たない。
class MotionControllerBase
{
public:
    virtual ~MotionControllerBase() = default;

    // CharacterBase::AddMotionControllerから一度だけ設定する。
    void Init(PhysicsBody* body) { physicsBody = body; }

    // 時間倍率は1フレーム分。接地判定は派生Controllerが管理する。
    virtual void ApplyInput(const Vector3& direction, float timeScale = TimeScale::Get()) = 0;

    // 入力がないステートでも毎フレーム呼び出す移動制御。
    virtual void Update(float timeScale) {}

    // 今の目標の移動速度(1フレームあたり)。移動しない制御は0。
    // 段差を登った直後に、壁で打ち消された速度を取り戻して、止まらずに進ませるために使う。
    virtual float GetCruiseSpeed(void) const { return 0.0f; }

    // 直近のApplyInputで渡された入力の向きを取り出す(取り出すと消える)。入力がなければfalse。
    // 壁に斜めに押しつけて、壁に沿って滑っている時も、進みたい向きで段差を判定するために使う。
    bool TakeLastInput(Vector3& out) {
        if (!hasLastInput) return false;
        out = lastInput;
        hasLastInput = false;
        return true;
    }
protected:

    // ApplyInputの先頭で呼び、入力の向きを記録する。
    void RecordInput(const Vector3& direction) {
        lastInput = direction;
        hasLastInput = true;
    }

    // 物理挙動制御の参照
    PhysicsBody* physicsBody = nullptr;

private:

    // 直近の入力の向き
    Vector3 lastInput = Vector3(0.0f);
    bool hasLastInput = false;
};
