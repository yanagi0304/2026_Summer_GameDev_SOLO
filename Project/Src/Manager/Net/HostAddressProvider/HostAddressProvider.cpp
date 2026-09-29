#include "HostAddressProvider.h"

#include <array>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>

HostAddressProvider::HostAddressProvider(MODE mode, unsigned short roomNumber, ENetHost* enetHost, enet_uint16 localPort) :
	mode(mode),
	state(STATE::Initialize),

	roomNumber(roomNumber),

	machineId(),

	enetHost(enetHost),

	controlSocket(-1),

	localEndpoint(),
	publicEndpoint(),

	hostMachineId(),
	hostLocalEndpoint(),
	hostPublicEndpoint(),

	connectEndpoint(),
	connectionRoute(CONNECTION_ROUTE::None),

	serverSendCounter(0),
	punchSendCounter(0),
	punchCount(0)
{
	// ENetHostチェック
	if (this->enetHost == nullptr) { state = STATE::Error; return; }

	// Machine ID取得
	machineId = GetOrCreateMachineId();

	if (machineId.empty()) { state = STATE::Error; return; }

	// LAN内IPv4取得
	localEndpoint.ip = GetLocalIPv4();

	if (localEndpoint.ip.empty()) { state = STATE::Error; return; }

	// NetWorkManager側で取得した
	// ENet自身のポート番号
	localEndpoint.port = localPort;

	if (localEndpoint.port == 0) {
		state = STATE::Error;
		return;
	}

	// Oracle制御通信用UDPソケット
	// 0 = OSに空きポートを選ばせる
	controlSocket = MakeUDPSocket(0);

	if (controlSocket == -1) { state = STATE::Error; return; }

	// 初期化完了
	state = STATE::Register;
}


//==============================================================
// 更新
//==============================================================
void HostAddressProvider::Update(void)
{
	if (state == STATE::Complete || state == STATE::Error) { return; }

	// Oracleからのメッセージを毎フレーム確認
	ReceiveServerMessage();

	// MODE別更新
	switch (mode) {

	case MODE::Host:
		HostUpdate();
		break;

	case MODE::Client:
		ClientUpdate();
		break;

	default:
		state = STATE::Error;
		break;
	}
}


//==============================================================
// 終了
//==============================================================
void HostAddressProvider::End(void)
{
	if (controlSocket != -1) {
		DeleteUDPSocket(controlSocket);
		controlSocket = -1;
	}

	// enetHostはNetWorkManager所有なので
	// ここでは破棄しない
	enetHost = nullptr;
}


//==============================================================
// 最終接続先取得
//==============================================================
bool HostAddressProvider::GetConnectEndpoint(
	NetEndpoint& endpoint,
	CONNECTION_ROUTE& route
) const
{
	if (state != STATE::Complete) {
		return false;
	}

	if (!connectEndpoint.IsValid()) {
		return false;
	}

	endpoint = connectEndpoint;
	route = connectionRoute;

	return true;
}


//==============================================================
// ホスト更新
//==============================================================
void HostAddressProvider::HostUpdate(void)
{
	switch (state) {

		//----------------------------------------------------------
		// Oracleへホスト登録
		//----------------------------------------------------------
	case STATE::Register:
	{
		// 一定間隔で再送する
		if (serverSendCounter <= 0) {

			RegisterToServer();

			// ENet自身のソケットからもOracleへ送る
			SendEndpointProbe();

			serverSendCounter = SERVER_SEND_COOLTIME;
		}
		else {
			--serverSendCounter;
		}

		break;
	}

	//----------------------------------------------------------
	// HOSTは基本的にマッチング待ち
	//----------------------------------------------------------
	case STATE::WaitMatch:
	{
		// KeepAlive兼再登録
		if (serverSendCounter <= 0) {

			RegisterToServer();

			SendEndpointProbe();

			serverSendCounter = SERVER_SEND_COOLTIME;
		}
		else {
			--serverSendCounter;
		}

		break;
	}

	//----------------------------------------------------------
	// NAT Hole Punch
	//----------------------------------------------------------
	case STATE::Punch:
	{
		if (!hostPublicEndpoint.IsValid()) {
			// HOST側ではこの変数に
			// 「接続してくるCLIENTのPublic Endpoint」が
			// 一時的に設定される
			break;
		}

		if (punchSendCounter <= 0) {

			SendPunchPacket(hostPublicEndpoint);

			punchSendCounter = PUNCH_SEND_COOLTIME;

			++punchCount;
		}
		else {
			--punchSendCounter;
		}

		if (punchCount >= PUNCH_MAX_COUNT) {

			//--------------------------------------------------
			// HOST側は接続先へconnectするわけではないため、
			// Hole Punchを十分送ったら通常待機へ戻る
			//--------------------------------------------------
			state = STATE::WaitMatch;

			punchCount = 0;
			punchSendCounter = 0;
		}

		break;
	}

	default:
		break;
	}
}


//==============================================================
// クライアント更新
//==============================================================
void HostAddressProvider::ClientUpdate(void)
{
	switch (state) {

		//----------------------------------------------------------
		// Oracleへ検索要求
		//----------------------------------------------------------
	case STATE::Register:
	{
		if (serverSendCounter <= 0) {

			RegisterToServer();

			//--------------------------------------------------
			// ENet自身のPublic EndpointもOracleに観測させる
			//--------------------------------------------------
			SendEndpointProbe();

			serverSendCounter = SERVER_SEND_COOLTIME;
		}
		else {
			--serverSendCounter;
		}

		break;
	}

	//----------------------------------------------------------
	// ホスト情報待ち
	//----------------------------------------------------------
	case STATE::WaitMatch:
	{
		//------------------------------------------------------
		// 必要情報がそろったら接続方法決定
		//------------------------------------------------------
		if (!hostMachineId.empty() &&
			hostLocalEndpoint.IsValid() &&
			hostPublicEndpoint.IsValid() &&
			publicEndpoint.IsValid())
		{
			DecideConnectionRoute();
		}

		//------------------------------------------------------
		// まだならOracleへ再問い合わせ
		//------------------------------------------------------
		if (state == STATE::WaitMatch) {

			if (serverSendCounter <= 0) {

				RegisterToServer();

				SendEndpointProbe();

				serverSendCounter = SERVER_SEND_COOLTIME;
			}
			else {
				--serverSendCounter;
			}
		}

		break;
	}

	//----------------------------------------------------------
	// NAT Hole Punch
	//----------------------------------------------------------
	case STATE::Punch:
	{
		if (!hostPublicEndpoint.IsValid()) {
			break;
		}

		if (punchSendCounter <= 0) {

			//--------------------------------------------------
			// ENet自身のソケットから
			// ホストのPublic EndpointへUDPを送る
			//--------------------------------------------------
			SendPunchPacket(hostPublicEndpoint);

			punchSendCounter = PUNCH_SEND_COOLTIME;

			++punchCount;
		}
		else {
			--punchSendCounter;
		}

		//------------------------------------------------------
		// 数回送ればENet接続へ移る
		//
		// 60回全部待つ必要はない。
		// 最初に数発穴を開ければ十分。
		//------------------------------------------------------
		if (punchCount >= 6) {

			connectEndpoint = hostPublicEndpoint;
			connectionRoute = CONNECTION_ROUTE::Public;

			state = STATE::Complete;
		}

		break;
	}

	default:
		break;
	}
}


//==============================================================
// Oracleへ登録
//==============================================================
void HostAddressProvider::RegisterToServer(void)
{
	std::ostringstream ss;

	//----------------------------------------------------------
	// HOST
	//
	// HOST <room> <machineId> <privateIp> <gamePort>
	//----------------------------------------------------------
	if (mode == MODE::Host) {

		ss
			<< "HOST "
			<< roomNumber << ' '
			<< machineId << ' '
			<< localEndpoint.ip << ' '
			<< localEndpoint.port;
	}

	//----------------------------------------------------------
	// CLIENT
	//
	// FIND <room> <machineId> <privateIp> <gamePort>
	//----------------------------------------------------------
	else {

		ss
			<< "FIND "
			<< roomNumber << ' '
			<< machineId << ' '
			<< localEndpoint.ip << ' '
			<< localEndpoint.port;
	}

	SendControlMessage(ss.str());
}


//==============================================================
// Oracleから受信
//==============================================================
void HostAddressProvider::ReceiveServerMessage(void)
{
	if (controlSocket == -1) { return; }

	// 1フレームで複数受信できるようにする
	for (int receiveCount = 0; receiveCount < 16; receiveCount++) {
		IPDATA senderAddress{};

		int senderPort = 0;

		std::array<char, 1024> buffer{};

		const int recvSize = NetWorkRecvUDP(controlSocket, &senderAddress, &senderPort, buffer.data(), static_cast<int>(buffer.size() - 1), 0);

		if (recvSize < 0) { break; }

		buffer[(static_cast<size_t>(recvSize) < buffer.size()) ? static_cast<size_t>(recvSize) : buffer.size() - 1] = '\0';

		const std::string message(buffer.data());

		std::istringstream ss(message);

		std::string command;

		ss >> command;

		// HOST登録成功
		if (command == "HOST_OK") {

			if (mode == MODE::Host) { state = STATE::WaitMatch; }

			continue;
		}

		// 自分のENet Public Endpoint
		if (command == "PUBLIC") {

			std::string ip;
			unsigned int port = 0;

			ss >> ip >> port;

			if (!ip.empty() && port > 0 && port <= 65535) {
				publicEndpoint.ip = ip;
				publicEndpoint.port = static_cast<enet_uint16>(port);

				if (mode == MODE::Client && state == STATE::Register) { state = STATE::WaitMatch; }
			}

			continue;
		}

		//------------------------------------------------------
		// ホスト発見
		//
		// FOUND
		// <machineId>
		// <privateIp>
		// <privatePort>
		// <publicIp>
		// <publicPort>
		//------------------------------------------------------
		if (command == "FOUND") {

			std::string machine;
			std::string localIp;
			std::string publicIp;

			unsigned int localPort = 0;
			unsigned int publicPort = 0;

			ss >> machine >> localIp >> localPort >> publicIp >> publicPort;

			if (machine.empty() || localIp.empty() || publicIp.empty() || localPort == 0 || localPort > 65535 || publicPort == 0 || publicPort > 65535) { continue; }

			hostMachineId = machine;

			hostLocalEndpoint.ip = localIp;
			hostLocalEndpoint.port = static_cast<enet_uint16>(localPort);

			hostPublicEndpoint.ip = publicIp;
			hostPublicEndpoint.port = static_cast<enet_uint16>(publicPort);

			if (mode == MODE::Client) { state = STATE::WaitMatch; }

			continue;
		}

		//------------------------------------------------------
		// NAT Hole Punch開始指示
		//
		// PUNCH <ip> <port>
		//------------------------------------------------------
		if (command == "PUNCH") {

			std::string ip;
			unsigned int port = 0;

			ss >> ip >> port;

			if (ip.empty() || port == 0 || port > 65535) { continue; }

			//--------------------------------------------------
			// HOST側の場合、
			// hostPublicEndpointを
			// 「CLIENTのPublic Endpoint」の一時格納として使う
			//--------------------------------------------------
			if (mode == MODE::Host) {

				hostPublicEndpoint.ip = ip;
				hostPublicEndpoint.port =
					static_cast<enet_uint16>(port);

				punchCount = 0;
				punchSendCounter = 0;

				state = STATE::Punch;
			}

			//--------------------------------------------------
			// CLIENT側の場合は
			// 基本FOUNDで取得済みだが、
			// OracleのPUNCH情報を優先する
			//--------------------------------------------------
			else {

				hostPublicEndpoint.ip = ip;
				hostPublicEndpoint.port =
					static_cast<enet_uint16>(port);

				punchCount = 0;
				punchSendCounter = 0;

				state = STATE::Punch;
			}

			continue;
		}

		//------------------------------------------------------
		// 部屋なし
		//
		// NOT_FOUND
		//
		// エラーにはせず検索を継続する
		//------------------------------------------------------
		if (command == "NOT_FOUND") {
			continue;
		}

		//------------------------------------------------------
		// 明示的エラー
		//------------------------------------------------------
		if (command == "ERROR") {

			state = STATE::Error;

			continue;
		}
	}
}


//==============================================================
// ENet自身のPublic EndpointをOracleに観測させる
//==============================================================
void HostAddressProvider::SendEndpointProbe(void)
{
	if (enetHost == nullptr) { return; }

	NetEndpoint server;

	server.ip = MATCH_SERVER_IP;
	server.port = static_cast<enet_uint16>(MATCH_SERVER_PORT);

	std::ostringstream ss;

	//----------------------------------------------------------
	// PROBE HOST 111 MachineId
	//
	// または
	//
	// PROBE CLIENT 111 MachineId
	//----------------------------------------------------------
	ss << "PROBE " << ((mode == MODE::Host) ? "HOST" : "CLIENT") << ' ' << roomNumber << ' ' << machineId << ' ' << localEndpoint.port;

	SendRawFromENet(server, ss.str());
}


//==============================================================
// NAT Hole Punch packet
//==============================================================
void HostAddressProvider::SendPunchPacket(
	const NetEndpoint& endpoint
)
{
	if (!endpoint.IsValid()) {
		return;
	}

	std::ostringstream ss;

	ss
		<< "PUNCH_PACKET "
		<< roomNumber
		<< ' '
		<< machineId;

	SendRawFromENet(
		endpoint,
		ss.str()
	);
}


//==============================================================
// 接続方法決定
//==============================================================
void HostAddressProvider::DecideConnectionRoute(void)
{
	//----------------------------------------------------------
	// ① 同じPC
	//----------------------------------------------------------
	if (hostMachineId == machineId) {

		connectEndpoint.ip = "127.0.0.1";
		connectEndpoint.port =
			hostLocalEndpoint.port;

		connectionRoute =
			CONNECTION_ROUTE::Loopback;

		state = STATE::Complete;

		return;
	}

	//----------------------------------------------------------
	// ② 同じグローバルIP
	//
	// 同一ルーター配下とみなし、
	// Private IPへ接続する。
	//----------------------------------------------------------
	if (publicEndpoint.IsValid() &&
		hostPublicEndpoint.IsValid() &&
		publicEndpoint.ip == hostPublicEndpoint.ip)
	{
		connectEndpoint =
			hostLocalEndpoint;

		connectionRoute =
			CONNECTION_ROUTE::Local;

		state = STATE::Complete;

		return;
	}

	//----------------------------------------------------------
	// ③ 別LAN
	//
	// Public Endpointへ直接接続する前に
	// UDP Hole Punchを行う。
	//----------------------------------------------------------
	if (hostPublicEndpoint.IsValid()) {

		punchCount = 0;
		punchSendCounter = 0;

		connectionRoute =
			CONNECTION_ROUTE::Public;

		state = STATE::Punch;

		return;
	}

	//----------------------------------------------------------
	// 情報不足
	//----------------------------------------------------------
	state = STATE::WaitMatch;
}


//==============================================================
// Machine ID
//==============================================================
std::string HostAddressProvider::GetOrCreateMachineId(void)
{
	//----------------------------------------------------------
	// 今回は
	//
	// PC名 + WindowsシステムドライブのVolumeSerial
	//
	// から作る。
	//
	// 同じPCで起動したexe同士なら同一になる。
	//----------------------------------------------------------

	char computerName[MAX_COMPUTERNAME_LENGTH + 1]{};

	DWORD computerNameLength =
		static_cast<DWORD>(
			std::size(computerName)
			);

	if (!GetComputerNameA(
		computerName,
		&computerNameLength))
	{
		return {};
	}

	DWORD volumeSerial = 0;

	if (!GetVolumeInformationA(
		"C:\\",
		nullptr,
		0,
		&volumeSerial,
		nullptr,
		nullptr,
		nullptr,
		0))
	{
		return {};
	}

	std::ostringstream ss;

	ss
		<< computerName
		<< '-'
		<< std::hex
		<< std::uppercase
		<< std::setw(8)
		<< std::setfill('0')
		<< volumeSerial;

	return ss.str();
}


//==============================================================
// LAN内IPv4取得
//==============================================================
std::string HostAddressProvider::GetLocalIPv4(void)
{
	char hostName[256]{};

	if (gethostname(
		hostName,
		static_cast<int>(sizeof(hostName))) != 0)
	{
		return {};
	}

	addrinfo hints{};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	addrinfo* result = nullptr;

	if (getaddrinfo(
		hostName,
		nullptr,
		&hints,
		&result) != 0)
	{
		return {};
	}

	std::string selectedAddress;

	for (addrinfo* ptr = result;
		ptr != nullptr;
		ptr = ptr->ai_next)
	{
		if (ptr->ai_family != AF_INET) {
			continue;
		}

		const sockaddr_in* ipv4 =
			reinterpret_cast<const sockaddr_in*>(
				ptr->ai_addr
				);

		char ipBuffer[INET_ADDRSTRLEN]{};

		if (inet_ntop(
			AF_INET,
			&ipv4->sin_addr,
			ipBuffer,
			sizeof(ipBuffer)) == nullptr)
		{
			continue;
		}

		std::string ip(ipBuffer);

		//------------------------------------------------------
		// Loopbackは除外
		//------------------------------------------------------
		if (ip.rfind("127.", 0) == 0) {
			continue;
		}

		//------------------------------------------------------
		// APIPAも除外
		//------------------------------------------------------
		if (ip.rfind("169.254.", 0) == 0) {
			continue;
		}

		selectedAddress = ip;

		//------------------------------------------------------
		// 192.168 / 10 / 172.16-31 を優先したいので、
		// 最初の有効IPv4を採用
		//------------------------------------------------------
		break;
	}

	freeaddrinfo(result);

	return selectedAddress;
}


//==============================================================
// Oracle制御メッセージ送信
//==============================================================
bool HostAddressProvider::SendControlMessage(
	const std::string& message
)
{
	if (controlSocket == -1) {
		return false;
	}

	if (message.empty()) {
		return false;
	}

	IPDATA serverAddress =
	{
		161,
		33,
		190,
		216
	};

	const int result =
		NetWorkSendUDP(
			controlSocket,
			serverAddress,
			MATCH_SERVER_PORT,
			message.c_str(),
			static_cast<int>(
				message.size() + 1
				)
		);

	return result >= 0;
}


//==============================================================
// ENet自身のソケットからRaw UDP送信
//==============================================================
bool HostAddressProvider::SendRawFromENet(
	const NetEndpoint& endpoint,
	const std::string& message
)
{
	if (enetHost == nullptr) {
		return false;
	}

	if (!endpoint.IsValid()) {
		return false;
	}

	if (message.empty()) {
		return false;
	}

	ENetAddress address{};

	if (enet_address_set_host(
		&address,
		endpoint.ip.c_str()) != 0)
	{
		return false;
	}

	address.port = endpoint.port;

	ENetBuffer buffer{};

	buffer.data =
		const_cast<char*>(
			message.data()
			);

	buffer.dataLength =
		message.size();

	const int result =
		enet_socket_send(
			enetHost->socket,
			&address,
			&buffer,
			1
		);

	return result >= 0;
}