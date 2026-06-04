#include "NetWorkManager.h"

NetWorkManager* NetWorkManager::ins = nullptr;

NetWorkManager::NetWorkManager() :
    state(NetState::None),
    stateUpdate(),

    hostAddressProvider(nullptr),

    senderId(MSG_SENDER_ID::None),
    connectInfo(),
    connectStatus(),

	host(nullptr),

    msgData()
{
}

#pragma region メイン処理
// 初期化
void NetWorkManager::Init()
{
    // 初期化
    enet_initialize();

#pragma region 各状態別更新処理関数を関数ポインタ配列に格納
    stateUpdate[(int)NetState::None] = &NetWorkManager::NoneUpdate;
    stateUpdate[(int)NetState::Hosting] = &NetWorkManager::HostingUpdate;
    stateUpdate[(int)NetState::Connecting] = &NetWorkManager::ConnectingUpdate;
    stateUpdate[(int)NetState::Connected] = &NetWorkManager::ConnectedUpdate;
    stateUpdate[(int)NetState::Disconnection] = &NetWorkManager::DisconnectionUpdate;
    stateUpdate[(int)NetState::Error] = &NetWorkManager::ErrorUpdate;
#pragma endregion
}
// 更新
void NetWorkManager::Update()
{
    // 生成されていたら
    if (hostAddressProvider) { hostAddressProvider->Update(); }
    // 状態に合わせた更新処理関数を呼び出す
    (this->*stateUpdate[(int)state])();
}
// 解放
void NetWorkManager::Release()
{
    // リセット
    DisconnectionComplete();

    enet_deinitialize();
}
#pragma endregion


#pragma region 各状態更新処理

// 未接続：何もしない
void NetWorkManager::NoneUpdate(void) {}

// ホストとして待機中
void NetWorkManager::HostingUpdate(void)
{
	// イベント受信用の一時変数
    ENetEvent event;

	// イベントがある限りループして処理する
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
			// 接続が確立したら、peerを保存して接続状況を更新する
            connectInfo.emplace_back(event.peer, connectStatus.AddMember());

			// 割り当てられた送信者IDを送る
            Send(MsgDataSenderId(connectInfo.back().senderId), this->senderId, connectInfo.back().senderId);

			// 接続状況を送る
            Send(MsgDataConnectStatus(connectStatus));
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            for (auto it = connectInfo.begin(); it != connectInfo.end(); ++it) {
                if (it->peer == event.peer) {
                    // 接続状況を更新（末尾を消す）
                    connectStatus.RemoveMember();
                    // 新しい接続状況を送る
                    Send(MsgDataConnectStatus(connectStatus));

					// 接続情報を消去
                    connectInfo.erase(it);
					// 新しい接続情報をもとにクライアントの送信IDを割り当てなおす
                    ReassignSenderId();

                    break;
                }
            }
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE: { MsgDataRecv(event); }
        }
    }
}

// クライアントとして接続中
void NetWorkManager::ConnectingUpdate(void)
{
    if (hostAddressProvider) {
        // ホストのIPアドレスを取得する
        std::string hostAddress;
        if (!hostAddressProvider->GetHostAddress(hostAddress)) { return; }

		// ホストアドレス取得クラスの削除
        hostAddressProvider->End();
        delete hostAddressProvider;
        hostAddressProvider = nullptr;

		// 接続要求を送る
        ENetAddress address;
        enet_address_set_host(&address, hostAddress.c_str());
        address.port = PORT_NUMBER;
        connectInfo.emplace_back(enet_host_connect(host, &address, (size_t)MSG_DATA_CHANNEL::Max, 0), HOST_SENDER_ID);
    }

    // イベント受信用の一時変数
    ENetEvent event;
    // イベントがある限りループして処理する
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) { continue; }
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) { DisconnectionComplete(); break; }
        MsgDataRecv(event);
    }
}

// 接続完了・プレイ中
void NetWorkManager::ConnectedUpdate(void)
{
	// イベント受信用の一時変数
    ENetEvent event;

	// イベントがある限りループして処理する
    while (enet_host_service(host, &event, 0) > 0) {
		// 接続通知
        if (event.type == ENET_EVENT_TYPE_CONNECT) { enet_peer_disconnect(event.peer, 0); continue; }

		// 切断通知
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            for (auto it = connectInfo.begin(); it != connectInfo.end(); it++) {
                if (it->peer != event.peer) { continue; }
                // 接続状況を更新
                connectStatus.LostMember(it->senderId);
                // 接続情報を消去
                connectInfo.erase(it);
                // 新しい接続状況を送る
                Send(MsgDataConnectStatus(connectStatus));
                break;
            }
            continue;
        }

		// 受信データを処理する
        MsgDataRecv(event);
    }
}

// 切断待ち
void NetWorkManager::DisconnectionUpdate(void)
{
    // イベント受信用の一時変数
    ENetEvent event;
    // 切断完了の通知を受け取るまでループして受け取ったらそのpeerを消去する（切断通知を送った側の処理）
    while (enet_host_service(host, &event, 100) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: { break; }
        case ENET_EVENT_TYPE_RECEIVE: { enet_packet_destroy(event.packet); break; }
        case ENET_EVENT_TYPE_DISCONNECT: {
            for (auto it = connectInfo.begin(); it != connectInfo.end(); it++) {
                // 接続情報を消去
                if (it->peer == event.peer) { connectInfo.erase(it); break; }
            }
            break;
        }
        }
    }

    // 以上の処理を接続情報がなくなるまで繰り返す
    if (!connectInfo.empty()) { return; }

    // 最終切断処理
	DisconnectionComplete();
}

// エラーや切断
void NetWorkManager::ErrorUpdate(void)
{
    // ホスト
    if (IsHost()) {
        // 状態を戻す
        state = NetState::Connected;
    }
    // クライアント
    else { DisconnectionComplete(); }
}

#pragma endregion