#pragma once

#include "../../pch.h"

#include "../../Common/Vector3.h"

#include "../Input/InputManager.h"

#include "SenderID_Define.h"

// データタイプ（<> <- 送信者）
enum class MSG_DATA_TYPE
{
    // 未設定
    None = -1,

    // <ホスト/クライアント>接続に関するシステムイベント（接続完了、切断など）
    ConnectInform,

    // <ホスト>ID
    SenderId,

    // <ホスト>接続状況
    ConnectStatus,

    // <ホスト>システムイベント（シーン遷移など）
    SystemInform,

	Max
};

// 送信チャンネル
enum class MSG_DATA_CHANNEL {
	None = -1,  // 未設定

	Reliable,   // 確実に届ける
	Unreliable, // 速度重視

    Max
};

// 送信チャンネルごとの送信保証フラグ
static constexpr enet_uint8 CHANNEL_PACKET_FLAG[(int)MSG_DATA_CHANNEL::Max] = {
    ENET_PACKET_FLAG_RELIABLE,
    0,
};

// 接続情報構造体
struct ConnectInfo { ENetPeer* peer; MSG_SENDER_ID senderId; };

// 接続状況
struct ConnectStatus {

private:
    // 参加状況 / 接続状況 マスク
    unsigned char entryMask = 1;
    unsigned char aliveMask = 1;

public:

#pragma region 取得系
    // 参加している総人数（切断中も含む）
    int EntryCount(void) const {
        int count = 0;
        for (int i = 0; i < (int)MSG_SENDER_ID::Max; i++) { if (entryMask & (1 << i)) count++; }
        return count;
    }

    // 特定のIDが「参加」しているか（切断中も含む）
    bool IsEntry(MSG_SENDER_ID id) const {
        if (id <= MSG_SENDER_ID::None || MSG_SENDER_ID::Max <= id) { return false; }
        return (entryMask & (1 << (int)id)) != 0;
    }

    // 特定のIDが「現在生存」しているか
    bool IsAlive(MSG_SENDER_ID id) const {
        if (id <= MSG_SENDER_ID::None || MSG_SENDER_ID::Max <= id) { return false; }
        return (aliveMask & (1 << (int)id)) != 0;
    }
#pragma endregion

#pragma region 設定系
    // メンバーを追加（参加と生存を同時にON）
    void AddMember(MSG_SENDER_ID id) {
        if (id <= MSG_SENDER_ID::None || MSG_SENDER_ID::Max <= id) { return; }
        unsigned char bit = (1 << (int)id);
        entryMask |= bit;
        aliveMask |= bit;
    }
    // メンバーを末尾に追加（参加と生存を同時にON）
    MSG_SENDER_ID AddMember(void) {
        // P1から順番にチェックして、entryMaskが立っていない（空いている）最初の枠を探す
        for (int i = 0; i < (int)MSG_SENDER_ID::Max; i++) {
            if (!IsEntry((MSG_SENDER_ID)i)) {
                AddMember((MSG_SENDER_ID)i);
                return (MSG_SENDER_ID)i;
            }
        }
        // 空きがない場合
        return MSG_SENDER_ID::None;
    }

    // 末尾のメンバーを完全に削除（最初からいなかったことにする）
    MSG_SENDER_ID RemoveMember(void) {
        // 後ろ（Max-1）から逆順に探す
        for (int i = (int)MSG_SENDER_ID::Max - 1; i >= 0; i--) {
            if (IsEntry((MSG_SENDER_ID)i)) {
                entryMask &= ~(1 << i);
                aliveMask &= ~(1 << i);

                return (MSG_SENDER_ID)i;
            }
        }
        return MSG_SENDER_ID::None;
    }

    // 通信切断のみ（参加状態は維持、生存だけOFF）
    void LostMember(MSG_SENDER_ID id) {
        if (id <= MSG_SENDER_ID::None || MSG_SENDER_ID::Max <= id) { return; }

        aliveMask &= ~(1 << (int)id);
    }

    // <復帰>「参加状態がON」「生存状態がOFF」の枠を探して、あれば復帰させる（どのIDとして復帰したかを返す）
    MSG_SENDER_ID RecoverMember(void) {
		// 参加状態がONで、生存状態がOFFの最初の枠を探す（P1から順番に）
        for (int i = 0; i < (int)MSG_SENDER_ID::Max; i++) {
            // 参加状態を見る
            if (!IsEntry((MSG_SENDER_ID)i)) { continue; }
            // 生存状態を見る
            if (IsAlive((MSG_SENDER_ID)i)) { continue; }

            // 条件を満たす最初の枠が見つかったら、そのIDを復帰させる
            AddMember((MSG_SENDER_ID)i);
            // 復帰させたIDを返す
            return (MSG_SENDER_ID)i;
        }
		// 条件を満たす枠がない場合
        return MSG_SENDER_ID::None;
    }

    // リセット（ホストだけの状態にする）
    void Reset(void) { entryMask = aliveMask = 1; }
#pragma endregion
};

#pragma region 各送信構造体定義

// ヘッダー（全ての構造体の先頭に配置する）
struct MsgDataHeader
{
    MSG_DATA_TYPE dataType;
    MSG_SENDER_ID senderId;

    MsgDataHeader(MSG_DATA_TYPE dataType) :
        dataType(dataType),
        senderId(MSG_SENDER_ID::None)
    {
    }
};

// <ホスト/クライアント>接続に関するシステムイベント送信構造体
struct MsgDataConnectInform
{
    // 列挙型定義との紐づけ
    static constexpr MSG_DATA_TYPE DATA_TYPE = MSG_DATA_TYPE::ConnectInform;

    // データの送信チャンネル
    static constexpr MSG_DATA_CHANNEL DATA_CHANNEL = MSG_DATA_CHANNEL::Reliable;

    // ヘッダー（全ての構造体の先頭に配置する）
    MsgDataHeader header;

    // 接続イベント列挙型定義
    enum class INFORM_TYPE
    {
        None = -1,

        Connect,                            // 接続

        Disconnect,                         // 切断

        CloseReceptionToConnected,          // <ホスト>接続待ち終了（接続人数確定）

        ResumptionReceptionToConnected,      // <ホスト>接続待ち再開
    };

	// 接続イベントの内容
    INFORM_TYPE inform;

    MsgDataConnectInform(INFORM_TYPE inform) :
        header(DATA_TYPE),
        inform(inform)
    {
    }
    MsgDataConnectInform(void) :
        header(DATA_TYPE),
        inform(INFORM_TYPE::None)
    {
    }
};

// <ホスト>ID送信構造体
struct MsgDataSenderId
{
    // 列挙型定義との紐づけ
    static constexpr MSG_DATA_TYPE DATA_TYPE = MSG_DATA_TYPE::SenderId;

    // データの送信チャンネル
    static constexpr MSG_DATA_CHANNEL DATA_CHANNEL = MSG_DATA_CHANNEL::Reliable;

    MsgDataHeader header;
    MSG_SENDER_ID senderId;

    MsgDataSenderId(MSG_SENDER_ID senderId) :
        header(DATA_TYPE),
        senderId(senderId)
    {
    }
    MsgDataSenderId(void) :
        header(DATA_TYPE),
        senderId(MSG_SENDER_ID::None)
    {
    }
};

// <ホスト>接続状況送信構造体
struct MsgDataConnectStatus
{
    // 列挙型定義との紐づけ
    static constexpr MSG_DATA_TYPE DATA_TYPE = MSG_DATA_TYPE::ConnectStatus;

    // データの送信チャンネル
    static constexpr MSG_DATA_CHANNEL DATA_CHANNEL = MSG_DATA_CHANNEL::Reliable;

    // ヘッダー（全ての構造体の先頭に配置する）
    MsgDataHeader header;

    ConnectStatus connectStatus;

    MsgDataConnectStatus(ConnectStatus connectStatus) :
        header(DATA_TYPE),
        connectStatus(connectStatus)
    {
    }
    MsgDataConnectStatus(void) :
        header(DATA_TYPE),
        connectStatus()
    {
    }
};

// <ホスト>システムイベント送信構造体
struct MsgDataSystemInform
{
    // 列挙型定義との紐づけ
    static constexpr MSG_DATA_TYPE DATA_TYPE = MSG_DATA_TYPE::SystemInform;

    // データの送信チャンネル
    static constexpr MSG_DATA_CHANNEL DATA_CHANNEL = MSG_DATA_CHANNEL::Reliable;

    // ヘッダー（全ての構造体の先頭に配置する）
    MsgDataHeader header;

    // システムイベント列挙型定義
    enum class INFORM_TYPE
    {
        None = -1,

		// 接続完了（ホストが接続待ちを終了して、接続人数が確定した）
		ConnectComplete,

        // タイトルシーンへ遷移
        ChangeSceneTitle,
        // ゲームシーンへ遷移
        ChangeSceneGame,
        JumpSceneGame,
        // クリアシーンへ遷移
        ChangeSceneClear,
    };

    INFORM_TYPE inform;

    MsgDataSystemInform(INFORM_TYPE inform) :
        header(DATA_TYPE),
        inform(inform)
    {
    }
    MsgDataSystemInform(void) :
        header(DATA_TYPE),
        inform(INFORM_TYPE::None)
    {
    }
};