#pragma once

#include "../../../pch.h"

#include <string>

class HostAddressProvider
{
public:
	enum class MODE { Host, Client, Max };

	HostAddressProvider(MODE mode);
	~HostAddressProvider() = default;

	// 更新処理
	void Update(void);

	// 終了
	void End(void) {
		// UDPソケットを削除する
		DeleteUDPSocket(udpSocket);
	}

	// <クライアント>ホストアドレス取得
	bool GetHostAddress(IPDATA& hostAddress) {
		// まだ検索中
		if (this->hostAddress.d1 == 0 && this->hostAddress.d2 == 0 && this->hostAddress.d3 == 0 && this->hostAddress.d4 == 0) { return false; }
		// アウトプット用の引数からアドレスを渡す
		hostAddress = this->hostAddress;
		// 取得完了
		return true;
	}
	bool GetHostAddress(std::string& hostAddress) {
		// まだ検索中
		if (this->hostAddress.d1 == 0 && this->hostAddress.d2 == 0 && this->hostAddress.d3 == 0 && this->hostAddress.d4 == 0) { return false; }
		// アウトプット用の引数からアドレスを渡す
		hostAddress.clear();
		hostAddress += std::to_string(this->hostAddress.d1).c_str();
		hostAddress += '.';
		hostAddress += std::to_string(this->hostAddress.d2).c_str();
		hostAddress += '.';
		hostAddress += std::to_string(this->hostAddress.d3).c_str();
		hostAddress += '.';
		hostAddress += std::to_string(this->hostAddress.d4).c_str();
		// 取得完了
		return true;
	}

	// ソケット生成 成功/失敗
	bool SocketCreateResult(void)const { return udpSocket != -1; }

private:
	// ブロードキャスト送信用のポート番号
	static constexpr int BROADCAST_PORT_NUMBER = 54322;

	// 全員一斉送信用のIPを用意
	const IPDATA BROADCAST_IP = { 255, 255, 255, 255 };

	// 送る信号
	const char* CONNECT_SIGNAL = "Casalist";

	// UDPソケット
	int udpSocket;

	// ホストのIPアドレス
	IPDATA hostAddress;

	// ホストが信号を送る間隔
	const int BROADCAST_SEND_COOLTIME = 100;
	// ホストが信号を送る間隔を管理するカウンター
	int broadCastSendCounter;

	// ホスト更新処理
	void HostUpdate(void);
	// クライアント更新処理
	void ClientUpdate(void);
	// モードごとの更新処理を格納する関数ポインタ配列
	void (HostAddressProvider::* modeUpdate)(void);
};