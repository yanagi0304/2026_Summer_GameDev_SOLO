#pragma once

#include "../../pch.h"

#include <vector>

#include <deque>

#include "NetWorkDefine.h"

#include "HostAddressProvider/HostAddressProvider.h"

class NetWorkManager
{
private:
	// インスタンス
	static NetWorkManager* ins;

	// コンストラクタ/デストラクタ
	NetWorkManager();
	~NetWorkManager() = default;

	static constexpr int PORT_NUMBER = 54321;

public:
#pragma region シングルトン定義
	// 生成/初期化
	static void CreateIns(void) { if (ins == nullptr) { ins = new NetWorkManager(); ins->Init(); } }
	// 取得
	static NetWorkManager& GetIns(void) { return *ins; }
	// 終了/削除
	static void DeleteIns(void) { if (ins != nullptr) { ins->Release(); delete ins; } }

	// コピー・ムーブ操作を禁止
	NetWorkManager(const NetWorkManager&) = delete;
	NetWorkManager& operator=(const NetWorkManager&) = delete;
	NetWorkManager(NetWorkManager&&) = delete;
	NetWorkManager& operator=(NetWorkManager&&) = delete;
#pragma endregion

	// ホストの送信ID
	static constexpr MSG_SENDER_ID HOST_SENDER_ID = MSG_SENDER_ID::P1;

	// 接続状態の列挙型定義
	enum class NetState {
		None,				// 未接続

		Hosting,			// ホストとして待機中
		Connecting,			// クライアントとして接続中

		Connected,			// 接続完了・プレイ中

		Disconnection,		// 切断待ち

		Error,				// エラー

		Max,
	};

	// 更新処理
	void Update(void);

#pragma region 共通操作
	/// <summary>データを送信</summary>
	/// <param name="data">送信データ構造体</param>
	/// <param name="senderId">送信者ID（指定なしで自動で自身のものが割り当てられる）</param>
	/// <param name="peer">送信先（指定なしで接続者全員に送信される）</param>
	template<typename MsgData>
	void Send(const MsgData& data, MSG_SENDER_ID senderId = MSG_SENDER_ID::None, MSG_SENDER_ID peer = MSG_SENDER_ID::None)const {
		// 接続先があるか
		if (connectInfo.empty()) { return; }

		// 受け取ったデータをコピーして送信者を設定
		MsgData sendData = data;
		sendData.header.senderId = (senderId == MSG_SENDER_ID::None) ? this->senderId : senderId;

		// 送信先が指定されていなければ、接続者全員に送信する
		// 送信先が指定されている場合は、その相手にのみ送信する
		for (const ConnectInfo& info : connectInfo) {
			if (info.peer == nullptr) { continue; }
			if (peer != MSG_SENDER_ID::None && info.senderId != peer) { continue; }

			// 送信データでパケットを作成
			ENetPacket* packet;
			packet = enet_packet_create(&sendData, sizeof(MsgData), CHANNEL_PACKET_FLAG[(int)MsgData::DATA_CHANNEL]);

			// 送信
			enet_peer_send(info.peer, (enet_uint8)MsgData::DATA_CHANNEL, packet);
		}
	}

	// イベント通知を送信（Send()をラッピングした関数）
	void EventInformSend(MsgDataSystemInform::INFORM_TYPE eventInform)const { Send(MsgDataSystemInform(eventInform)); }

	// データを取得する
	template <typename DataType>
	DataType* GetMsgData(MSG_SENDER_ID senderId = HOST_SENDER_ID) {
		// 参照を取得
		std::deque<void*>& queue = msgData[(int)DataType::DATA_TYPE][(int)senderId];

		// 受信データがあるかどうか
		if (queue.empty()) { return nullptr; }

		// データのポインタをコピーする
		DataType* data = static_cast<DataType*>(queue.front());

		// 配列から削除する
		queue.pop_front();

		// 返す
		return data;
	}

	// 状態取得
	NetState GetState(void)const { return state; }

	// ホストかどうか
	bool IsHost(void)const { return senderId == HOST_SENDER_ID; }

	// 自身の送信ID
	MSG_SENDER_ID GetSenderId(void)const { return senderId; }

	// 接続状況を取得
	const ConnectStatus& GetConnectStatus(void)const { return connectStatus; }

	// 切断要求を送る（ホスト・クライアント共通）
	void Disconnection(void) {
		if (state == NetState::None) { return; }

		// 切断要求を全員に送る
		for (ConnectInfo& info : connectInfo) {
			if (!info.peer) { continue; }
			enet_peer_disconnect(info.peer, 0);
		}
		// 状態を「切断待ち」に遷移させる
		state = NetState::Disconnection;
	}

#pragma endregion 共通操作


#pragma region ホスト操作
	// ホストとして受付開始
	bool StartHost(void) {
		// 接続受付を開始する
		ENetAddress address = ENetAddress(ENET_HOST_ANY, PORT_NUMBER);
		host = enet_host_create(&address, (size_t)MSG_SENDER_ID::Max, (size_t)MSG_DATA_CHANNEL::Max, 0, 0);

		// セッティング失敗として「false」を返し終了
		if (host == nullptr) { return false; }

		// 成功～～～～～～～～～～～～～～～～～～～～～～～

		// 状態を「ホストとして待機中」に設定する
		state = NetState::Hosting;

		// 送信IDを設定
		senderId = HOST_SENDER_ID;

		// 接続状況を更新
		connectStatus.Reset();

		// ブロードキャスト送信のためのUDPソケットを生成する
		hostAddressProvider = new HostAddressProvider(HostAddressProvider::MODE::Host);

		// ウィンドウテキストを設定
		SetWindowText("<ホスト> P1");

		// セッティング成功として「true」を返し終了
		return true;
	}

	// 接続受付を終了し、「接続完了・プレイ中」の状態へ遷移する
	void CloseReceptionToConnected(void) {
		// ホストアドレス取得クラスの削除
		hostAddressProvider->End();
		delete hostAddressProvider;
		hostAddressProvider = nullptr;

		// クライアントに接続人数確定（接続待ち終了）の通知を送る
		Send(MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::CloseReceptionToConnected));

		// 状態を「接続完了・プレイ中」に設定
		state = NetState::Connected;
	}
#pragma endregion ホスト操作


#pragma region クライアント操作

	// クライアントとして接続
	void ConnectClient(void) {
		bool socketCreate = false;
		while (!socketCreate) {
			hostAddressProvider = new HostAddressProvider(HostAddressProvider::MODE::Client);
			socketCreate = hostAddressProvider->SocketCreateResult();
			if (!socketCreate) {
				hostAddressProvider->End();
				delete hostAddressProvider;
				hostAddressProvider = nullptr;
			}
		}

		host = enet_host_create(nullptr, 1, (size_t)MSG_DATA_CHANNEL::Max, 0, 0);

		// 状態を「クライアントとして接続中」に設定する
		state = NetState::Connecting;
	}

#pragma endregion クライアント操作


private:
	// 初期化処理
	void Init(void);
	// 終了処理
	void Release(void);

	// 1フレームで受信できる最大情報数
	const char RECEPTION_MAX_NUM = 10;

	// 現在の状態
	NetState state;

#pragma region 各状態更新処理
	// 未接続(更新処理)
	void NoneUpdate(void);

	// ホストとして待機中(更新処理)
	void HostingUpdate(void);

	// クライアントとして接続中(更新処理)
	void ConnectingUpdate(void);
	
	// 接続完了・プレイ中(更新処理)
	void ConnectedUpdate(void);

	// 切断待ち(更新処理)
	void DisconnectionUpdate(void);

	// エラー(更新処理)
	void ErrorUpdate(void);

	// 上記各状態(更新処理)を格納する関数ポインタ配列
	void (NetWorkManager::*stateUpdate[(int)NetState::Max])(void);
#pragma endregion

	// ホストアドレス取得用
	HostAddressProvider* hostAddressProvider;

	// 送信ID
	MSG_SENDER_ID senderId;

	// ホスト
	ENetHost* host;
	// 接続情報
	std::vector<ConnectInfo>connectInfo;

	// 接続状況
	ConnectStatus connectStatus;

	// 受け取った情報のポインタを格納する配列
	std::deque<void*> msgData[(int)MSG_DATA_TYPE::Max][(int)MSG_SENDER_ID::Max];

#pragma region 受信処理
	// データの種類を振り分ける
	void MsgDataRecv(const ENetEvent& event) {
		// ヘッダー取得
		MsgDataHeader* headerData = (MsgDataHeader*)event.packet->data;

		// ヘッダーを参照してデータの種類を振り分ける
		switch (headerData->dataType) {
		case MSG_DATA_TYPE::None: { break; }
		case MSG_DATA_TYPE::ConnectInform: { MsgDataConnectInformRecv(event); break; }
		case MSG_DATA_TYPE::SenderId: { MsgDataSenderIdRecv(event); break; }
		case MSG_DATA_TYPE::ConnectStatus: { MsgDataConnectStatusRecv(event); break; }
		case MSG_DATA_TYPE::SystemInform: { MsgDataRecv<MsgDataSystemInform>(event, headerData->senderId); break; }
		case MSG_DATA_TYPE::PlayerTrans: { MsgDataRecv<MsgDataPlayerTrans>(event, headerData->senderId); break; }
		case MSG_DATA_TYPE::PlayerAnimeStep: { MsgDataRecv<MsgDataPlayerAnimeStep>(event, headerData->senderId); break; }
		case MSG_DATA_TYPE::PlayerInput: { MsgDataRecv<MsgDataPlayerInput>(event, headerData->senderId); break; }
		case MSG_DATA_TYPE::PlayerDamage: { MsgDataRecv<MsgDataPlayerDamage>(event, headerData->senderId); break; }
		}

		// パケットの解放
		enet_packet_destroy(event.packet);
	}

	// データのポインタを配列へ格納する関数
	template <typename DataType>
	void MsgDataRecv(const ENetEvent& event, MSG_SENDER_ID senderId)	{
		DataType* newData = new DataType();
		memcpy(newData, event.packet->data, sizeof(DataType));
		msgData[(int)DataType::DATA_TYPE][(int)senderId].emplace_back(newData);
	}

	// 接続に関するシステムイベントを受信して処理する
	void MsgDataConnectInformRecv(const ENetEvent& event) {
		MsgDataConnectInform* recvData = (MsgDataConnectInform*)event.packet->data;
		switch (recvData->inform) {
		case MsgDataConnectInform::INFORM_TYPE::CloseReceptionToConnected: {
			state = NetState::Connected;
			break; 
		}
		}
	}

	// 送信者IDを受信して設定
	void MsgDataSenderIdRecv(const ENetEvent& event) {
		MsgDataSenderId* recvData = (MsgDataSenderId*)event.packet->data;
		senderId = recvData->senderId;
		std::string playerNumber = "<クライアント> P";
		switch (senderId) {
		case MSG_SENDER_ID::P2: { playerNumber += "2"; break; }
		case MSG_SENDER_ID::P3: { playerNumber += "3"; break; }
		case MSG_SENDER_ID::P4: { playerNumber += "4"; break; }
		}
		SetWindowText(playerNumber.c_str());
	}

	// 接続状況を受信して設定
	void MsgDataConnectStatusRecv(const ENetEvent& event) {
		MsgDataConnectStatus* recvData = (MsgDataConnectStatus*)event.packet->data;
		connectStatus = recvData->connectStatus;
	}
#pragma endregion

	// <ホスト処理>クライアントの送信IDを1から割り当てなおす
	void ReassignSenderId(void) {
		int id = 0;

		// 接続情報を1つずつ参照
		for (ConnectInfo& info : connectInfo) {
			// 1つずつ送信IDを割り振る
			id++;
			info.senderId = (MSG_SENDER_ID)id;

			// クライアントに送信
			Send(MsgDataSenderId(info.senderId), this->senderId, info.senderId);
		}
	}

	// 接続完全リセット
	void DisconnectionComplete(void) {
		// ホストアドレス取得クラスの削除
		if (hostAddressProvider) {
			hostAddressProvider->End();
			delete hostAddressProvider;
			hostAddressProvider = nullptr;
		}

		// 格納保持されたままのたまった受信データを全て 解放/削除 する
		for (int dataType = 0; dataType < (int)MSG_DATA_TYPE::Max; dataType++) {
			for (int senderId = 0; senderId < (int)MSG_SENDER_ID::Max; senderId++) {
				for (void* ptr : msgData[dataType][senderId]) {
					if (!ptr) { continue; }
					switch ((MSG_DATA_TYPE)dataType) {
					case MSG_DATA_TYPE::SystemInform: { delete static_cast<MsgDataSystemInform*>(ptr); break; }
					case MSG_DATA_TYPE::PlayerTrans: { delete static_cast<MsgDataPlayerTrans*>(ptr); break; }
					case MSG_DATA_TYPE::PlayerInput: { delete static_cast<MsgDataPlayerInput*>(ptr); break; }
					case MSG_DATA_TYPE::PlayerDamage: { delete static_cast<MsgDataPlayerDamage*>(ptr); break; }
					}
				}
				msgData[dataType][senderId].clear();
			}
		}

		// 全切断
		for (ConnectInfo& info : connectInfo) {
			if (!info.peer) { continue; }
			enet_peer_reset(info.peer);
		}
		connectInfo.clear();
		// 接続状況をリセットする
		connectStatus.Reset();

		// ホスト解放
		if (host) {
			enet_host_flush(host);
			enet_host_destroy(host);
			host = nullptr;
		}

		// 送信者IDを未設定に戻す
		senderId = MSG_SENDER_ID::None;

		// 状態を「未接続」に戻す
		state = NetState::None;

		// ウィンドウテキストを消す
		SetWindowText("");
		clsDx();
	}
};
using Net = NetWorkManager;
using NetState = Net::NetState;