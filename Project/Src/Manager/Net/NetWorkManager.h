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

	/// <summary>
	/// データを取得する
	/// </summary>
	/// <param name="DataType">typename 構造体の種類</param>
	/// <param name="senderId">送信者ID（誰が送ったデータを取得するか）（指定なし(None)で全IDを探索、最初に見つかったデータを取得する）</param>
	/// <param name="reSend">取得したデータをホストがクライアントへ再分配するかどうか（「true」= する、「false」= しない）（デフォルトは「false」）</param>
	/// <param name="ownReSend">ホストが再分配する際、送信元のIDにも送るかどうか（1つ前の引数(reSend)指定が「true」である前提）（「true」= する、「false」= しない）（デフォルトは「false」）</param>
	/// <returns></returns>
	template <typename DataType>
	DataType* GetMsgData(MSG_SENDER_ID senderId = MSG_SENDER_ID::None, bool reSend = false, bool ownReSend = false) {

		// 送信者IDが指定されていない場合は、
		// 全ての送信者IDのデータを探索して最初に見つかったものを返す仕様にする
		if (senderId == MSG_SENDER_ID::None) {

			// 送信者IDを1から順に参照していく（ホストの送信IDは常に1なので、ホストのものが優先される）
			senderId = (MSG_SENDER_ID)((int)senderId + 1);

			// 送信者IDを順に参照していき、データが見つかったらループを抜ける
			for (; (int)senderId < (int)MSG_SENDER_ID::Max - 1; senderId = (MSG_SENDER_ID)((int)senderId + 1)) {
				if (!msgData[(int)DataType::DATA_TYPE][(int)senderId].empty()) { break; }
			}
		}

		// 参照を取得
		std::deque<void*>& queue = msgData[(int)DataType::DATA_TYPE][(int)senderId];

		// 受信データがあるかどうか
		if (queue.empty()) { return nullptr; }

		// データのポインタをコピーする
		DataType* data = static_cast<DataType*>(queue.front());

		// 配列から削除する
		queue.pop_front();

		// 自信がホストかつ引数(reSend)がtrueなら再分配する
		if (IsHost() && reSend) {
			for (int id = 1; id < (int)MSG_SENDER_ID::Max; id++) {
				if (id == (int)data->header.senderId && !ownReSend) { continue; }
				Send(*data, data->header.senderId, (MSG_SENDER_ID)id);
			}
		}

		// 返す
		return data;
	}

	// ホストかどうか
	bool IsHost(void)const { return GetSenderId() == HOST_SENDER_ID; }

	// 状態取得
	NetState GetState(void)const { return state; }

	// 自身の送信ID
	MSG_SENDER_ID GetSenderId(void)const { return (state != NetState::None) ? senderId : HOST_SENDER_ID; }

	// 接続状況を取得
	const ConnectStatus& GetConnectStatus(void)const { return connectStatus; }

	// あいことば設定
	unsigned short GetAddressProviderPassword(void)const { return addressProviderPassword; }
	// あいことば設定
	void SetAddressProviderPassword(unsigned short password) { addressProviderPassword = password; }

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

	// 格納保持されたままのデータを解放する
	void ReceptionDataReset(void) {
		// 格納保持されたままのたまった受信データを全て 解放/削除 する
		for (int dataType = 0; dataType < (int)MSG_DATA_TYPE::Max; dataType++) {
			for (int senderId = 0; senderId < (int)MSG_SENDER_ID::Max; senderId++) {
				for (void* ptr : msgData[dataType][senderId]) {
					if (!ptr) { continue; }
					switch ((MSG_DATA_TYPE)dataType) {
					case MSG_DATA_TYPE::None: { delete ptr; break; }
					case MSG_DATA_TYPE::ConnectInform: { delete static_cast<MsgDataConnectInform*>(ptr); break; }
					case MSG_DATA_TYPE::SenderId: { delete static_cast<MsgDataSenderId*>(ptr); break; }
					case MSG_DATA_TYPE::ConnectStatus: { delete static_cast<MsgDataConnectStatus*>(ptr); break; }
					case MSG_DATA_TYPE::SystemInform: { delete static_cast<MsgDataSystemInform*>(ptr); break; }
					}
				}
				msgData[dataType][senderId].clear();
			}
		}
	}
#pragma endregion 共通操作


#pragma region ホスト操作
	// ホストとして受付開始
	bool StartHost(void) {
		// すでにホストが存在している場合は失敗
		if (host != nullptr) { return false; }

		ENetAddress address{};
		address.host = ENET_HOST_ANY;

		// 0を指定するとOSが空いているポートを自動割り当て
		address.port = 0;

		// 生成
		host = enet_host_create(&address, (size_t)MSG_SENDER_ID::Max, (size_t)MSG_DATA_CHANNEL::Max, 0, 0);

		// 生成に失敗したら「操作失敗」として処理を中断する
		if (host == nullptr) { return false; }

		// 実際にOSから割り当てられたポート番号を取得
		ENetAddress localAddress{};
		if (enet_socket_get_address(host->socket, &localAddress) != 0) {
			enet_host_destroy(host);
			host = nullptr;

			return false;
		}

		// 実際にOSから割り当てられたポート番号を取得
		localGamePort = localAddress.port;

		// ホスト
		senderId = HOST_SENDER_ID;

		// 接続情報初期化
		connectInfo.clear();

		// 接続状態初期化
		connectStatus.Reset();

		// 新しいHostAddressProviderを生成
		hostAddressProvider = new HostAddressProvider(HostAddressProvider::MODE::Host, addressProviderPassword, host, localGamePort);

		// 状態を更新
		state = NetState::Hosting;

		// ウィンドウテキストをホストとして設定
		SetWindowText("<ホスト> P1");

		return true;
	}

	// 接続受付を終了し、「接続完了・プレイ中」の状態へ遷移する
	void CloseReceptionToConnected(void) {
		// ホストアドレス取得クラスの削除
		if (hostAddressProvider != nullptr) {
			hostAddressProvider->End();
			delete hostAddressProvider;
			hostAddressProvider = nullptr;
		}

		// クライアントに接続人数確定（接続待ち終了）の通知を送る
		Send(MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::CloseReceptionToConnected));

		// 状態を「接続完了・プレイ中」に設定
		state = NetState::Connected;
	}

	// 現状の接続を維持したまま、接続受付を再開する
	void ResumptionReceptionToHosting(void) {

		if (state != NetState::Connected) { return; }
		if (!IsHost()) { return; }

		// 保存情報でHostAddressProviderを生成
		hostAddressProvider = new HostAddressProvider(HostAddressProvider::MODE::Host, addressProviderPassword, host, localGamePort);

		// クライアントに接続人数確定（接続待ち終了）の通知を送る
		Send(MsgDataConnectInform(MsgDataConnectInform::INFORM_TYPE::ResumptionReceptionToConnected));

		// 状態を更新
		state = NetState::Hosting;
	}
#pragma endregion ホスト操作


#pragma region クライアント操作

	// クライアントとして接続
	void ConnectClient(void) {

		// 重複して呼ばれないように
		if (host != nullptr) { return; }

		// クライアント用ENetHost生成
		ENetAddress bindAddress{};
		// 全NICで受信可能
		bindAddress.host = ENET_HOST_ANY;
		// OSに空いているポートを自動で割り当ててもらう
		bindAddress.port = 0;

		// 生成
		host = enet_host_create(&bindAddress, 1, (size_t)MSG_DATA_CHANNEL::Max, 0, 0);

		// 生成失敗
		if (host == nullptr) { state = NetState::Error; return; }

		// OSから実際に割り当てられたポート番号を取得
		ENetAddress localAddress{};
		if (enet_socket_get_address(host->socket, &localAddress) != 0) {

			enet_host_destroy(host);
			host = nullptr;

			state = NetState::Error;

			return;
		}

		localGamePort = localAddress.port;

		// HostAddressProvider生成
		hostAddressProvider = new HostAddressProvider(HostAddressProvider::MODE::Client, addressProviderPassword, host, localGamePort);

		// SenderIDはホスト接続後に決定
		senderId = MSG_SENDER_ID::Max;

		// 接続中へ
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
	// アドレス取得用あいことば
	unsigned short addressProviderPassword;
	// OSから自動割り当てされた、ENetゲーム通信用ポート
	enet_uint16 localGamePort;

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
		case MSG_DATA_TYPE::ConnectInform: { MsgDataRecv<MsgDataConnectInform>(event, headerData->senderId); break; }
		case MSG_DATA_TYPE::SenderId: { MsgDataSenderIdRecv(event); break; }
		case MSG_DATA_TYPE::ConnectStatus: { MsgDataConnectStatusRecv(event); break; }
		case MSG_DATA_TYPE::SystemInform: { MsgDataRecv<MsgDataSystemInform>(event, headerData->senderId); break; }
		}

		// パケットの解放
		enet_packet_destroy(event.packet);
	}

	// データのポインタを配列へ格納する関数
	template <typename DataType>
	void MsgDataRecv(const ENetEvent& event, MSG_SENDER_ID senderId)	{

		// 受信データのサイズが構造体のサイズと一致しない場合は破棄する
		if (event.packet->dataLength != sizeof(DataType)) {
			enet_packet_destroy(event.packet);
			return;
		}

		// 受信データをコピーして配列に格納する
		DataType* newData = new DataType();
		memcpy(newData, event.packet->data, sizeof(DataType));
		msgData[(int)DataType::DATA_TYPE][(int)senderId].emplace_back(newData);
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
		if (hostAddressProvider != nullptr) {
			hostAddressProvider->End();
			delete hostAddressProvider;
			hostAddressProvider = nullptr;
		}

		// 格納保持されたままのたまった受信データを全て 解放/削除 する
		ReceptionDataReset();

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

		// ENetゲームポートを未設定に戻す
		localGamePort = 0;

		// 送信者IDを未設定に戻す
		senderId = MSG_SENDER_ID::None;

		// 状態を「未接続」に戻す
		state = NetState::None;

		// あいことばをリセット
		addressProviderPassword = 0;

		// ウィンドウテキストを消す
		SetWindowText("");
		clsDx();
	}
};
using Net = NetWorkManager;
using NetState = Net::NetState;