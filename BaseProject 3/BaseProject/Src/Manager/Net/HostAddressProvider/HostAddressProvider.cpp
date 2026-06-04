#include "HostAddressProvider.h"

HostAddressProvider::HostAddressProvider(MODE mode) :
	hostAddress(),

	broadCastSendCounter(0)
{
	// ソケットを生成
	udpSocket = MakeUDPSocket((mode == MODE::Host) ? 0 : BROADCAST_PORT_NUMBER);

	// 関数ポインタ設定
	modeUpdate =
		(mode == MODE::Host) ? &HostAddressProvider::HostUpdate :
		(mode == MODE::Client) ? &HostAddressProvider::ClientUpdate :
		nullptr;
}

void HostAddressProvider::Update(void)
{
	// モードに合わせた更新処理関数を呼び出す
	(this->*modeUpdate)();
}

void HostAddressProvider::HostUpdate(void)
{
	// ブロードキャスト送信(一定間隔で送り続ける)
	if (++broadCastSendCounter >= BROADCAST_SEND_COOLTIME) {
		broadCastSendCounter = 0;
		NetWorkSendUDP(udpSocket, BROADCAST_IP, BROADCAST_PORT_NUMBER, CONNECT_SIGNAL, sizeof(CONNECT_SIGNAL));
	}
}

void HostAddressProvider::ClientUpdate(void)
{
	// IPアドレス取得のための一時変数
	IPDATA hostAddress;

	// メモリ確保
	char buffer[256] = {};

	// 叫び声が届いているか確認
	if (NetWorkRecvUDP(udpSocket, &hostAddress, nullptr, buffer, sizeof(buffer), 0) >= 0) {
		if (strcmp(buffer, CONNECT_SIGNAL) == 0) {
			// ホストが見つかった場合、IPアドレスを保持
			this->hostAddress = hostAddress;
		}
	}
}