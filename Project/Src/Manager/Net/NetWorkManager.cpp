#include "NetWorkManager.h"

NetWorkManager* NetWorkManager::ins = nullptr;

NetWorkManager::NetWorkManager() :
    state(NetState::None),
    stateUpdate(),

    hostAddressProvider(nullptr),
    addressProviderPassword(0),
    localGamePort(0),

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
    // 状態に合わせた更新処理関数を呼び出す
    (this->*stateUpdate[(int)state])();
}
// 解放
void NetWorkManager::Release()
{
    if (state != NetState::None) {

        // 接続中の強制終了の場合、ここで切断要求を送る
        Disconnection();

        // 接続相手が切断要求を受け取らないままpeerを消去する恐れがあるため、
        // 切断完了の通知を受け取るまでここで待つ（無限ループ防止のため 1000回 のループを制限とする（60FPS基準でおよそ15秒程度））
        for (unsigned short i = 0; i < 1000; i++) {

            // イベント受信用の一時変数
            ENetEvent event;

            // 切断完了の通知を受け取ったらそのpeerを消去する
            while (enet_host_service(host, &event, 0) > 0) {
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

            // 接続情報が全てなくなったらループ終了
            if (connectInfo.empty()) { break; }
        }
    }

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
    if (hostAddressProvider != nullptr) {
        hostAddressProvider->Update();

        if (hostAddressProvider->IsError()) {
            hostAddressProvider->End();

            delete hostAddressProvider;
            hostAddressProvider = nullptr;
        }
    }

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

            // 新規接続としてキューに情報を保持する
            MsgDataConnectInform* connectInform = new MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::Connect);
            connectInform->header.senderId = connectInfo.back().senderId;
            msgData[(int)MsgDataConnectInform::DATA_TYPE][(int)connectInfo.back().senderId].emplace_back(connectInform);

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            for (auto it = connectInfo.begin(); it != connectInfo.end(); ++it) {
                if (it->peer == event.peer) {

                    // 切断があったことを保存する
                    MsgDataConnectInform* disConnectInfo = new MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::Disconnect);
                    disConnectInfo->header.senderId = it->senderId;
                    msgData[(int)MsgDataConnectInform::DATA_TYPE][(int)it->senderId].emplace_back(disConnectInfo);

                    // 接続情報を消去
                    connectInfo.erase(it);

					// 新しい接続情報をもとにクライアントの送信IDを割り当てなおす
                    ReassignSenderId();

                    // 接続状況を更新（末尾を消す）
                    connectStatus.RemoveMember();

                    // 新しい接続状況を送る
                    Send(MsgDataConnectStatus(connectStatus));

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
    // 接続先探索中
    if (hostAddressProvider != nullptr) {
        // Provider更新
        hostAddressProvider->Update();

        // エラー
        if (hostAddressProvider->IsError()) {
            hostAddressProvider->End();

            delete hostAddressProvider;
            hostAddressProvider = nullptr;

            state = NetState::Error;

            return;
        }

        // 最終接続先取得
        NetEndpoint endpoint;
        CONNECTION_ROUTE route;

        // まだ検索中
        if (!hostAddressProvider->GetConnectEndpoint(endpoint, route)) { return; }

        //------------------------------------------------------
        // Provider終了
        //------------------------------------------------------

        hostAddressProvider->End();
        delete hostAddressProvider;
        hostAddressProvider = nullptr;

        // ENet接続先作成
        ENetAddress address{};
        if (enet_address_set_host(&address, endpoint.ip.c_str()) != 0) {
            state = NetState::Error;
            return;
        }

        address.port = endpoint.port;

        // 接続要求
        ENetPeer* peer = enet_host_connect(host, &address, (size_t)MSG_DATA_CHANNEL::Max, 0);

        if (peer == nullptr) { state = NetState::Error; return; }
    }

    // ENetイベント処理

    ENetEvent event{};
    while (enet_host_service(host, &event, 0) > 0) {

        // 接続成功
        if (event.type == ENET_EVENT_TYPE_CONNECT) {

            // 状態を更新
            state = NetState::Connected;

            // 接続先を保存
            connectInfo.emplace_back(event.peer, HOST_SENDER_ID);

            // 新規接続確立を情報として保存
            MsgDataConnectInform* connectInform = new MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::Connect);
            connectInform->header.senderId = HOST_SENDER_ID;
            msgData[(int)MsgDataConnectInform::DATA_TYPE][(int)HOST_SENDER_ID].emplace_back(connectInform);

            continue;
        }

        // 切断
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) { DisconnectionComplete(); continue; }

        // データ受信
        if (event.type == ENET_EVENT_TYPE_RECEIVE) { MsgDataRecv(event); continue; }
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

            if (IsHost()) {
                for (auto it = connectInfo.begin(); it != connectInfo.end(); it++) {
                    if (it->peer != event.peer) { continue; }
                    // 接続状況を更新
                    connectStatus.LostMember(it->senderId);

                    // 切断があったことを保存する
                    MsgDataConnectInform* connectInform = new MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::Disconnect);
                    connectInform->header.senderId = it->senderId;
                    msgData[(int)MsgDataConnectInform::DATA_TYPE][(int)it->senderId].emplace_back(connectInform);

                    // 接続情報を消去
                    connectInfo.erase(it);
                    // 新しい接続状況を送る
                    Send(MsgDataConnectStatus(connectStatus));
                    break;
                }
            }
            else {

                // 切断があったことを保存する
                MsgDataConnectInform* connectInform = new MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::Disconnect);
                connectInform->header.senderId = HOST_SENDER_ID;
                msgData[(int)MsgDataConnectInform::DATA_TYPE][(int)HOST_SENDER_ID].emplace_back(connectInform);

                for (auto it = connectInfo.begin(); it != connectInfo.end(); it++) {
                    // 接続情報を消去
                    if (it->peer == event.peer) { connectInfo.erase(it); break; }
                }

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
    while (enet_host_service(host, &event, 0) > 0) {
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