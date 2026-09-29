#pragma once

#include "../../../pch.h"

#include <cstdint>
#include <string>

//==============================================================
// Ú‘±æ
//==============================================================
struct NetEndpoint
{
	std::string ip;
	enet_uint16 port = 0;

	bool IsValid(void) const
	{
		return !ip.empty() && port != 0;
	}
};

//==============================================================
// Ú‘±Œo˜H
//==============================================================
enum class CONNECTION_ROUTE
{
	None,

	// “¯‚¶PC
	Loopback,

	// “¯‚¶LAN
	Local,

	// ƒCƒ“ƒ^[ƒlƒbƒg‰z‚µ
	Public,

	// «—ˆ—pFƒŠƒŒ[
	Relay,
};

//==============================================================
// HostAddressProvider
//
// ƒzƒXƒg‚ÌuIP‚¾‚¯v‚ğ’T‚·ƒNƒ‰ƒX‚Å‚Í‚È‚­A
//
//   IP
//   Port
//   Ú‘±Œo˜H
//
// ‚ğŒˆ’è‚·‚éƒNƒ‰ƒXB
//==============================================================
class HostAddressProvider
{
public:

	// ƒ‚[ƒhiƒzƒXƒg or ƒNƒ‰ƒCƒAƒ“ƒgj
	enum class MODE { Host, Client, Max };

private:

	// XVó‘Ô
	enum class STATE {
		// ‰Šú‰»’¼Œã
		Initialize,

		// Oracle‚Ö©g‚Ìî•ñ‚ğ“o˜^
		Register,

		// ƒ}ƒbƒ`ƒ“ƒOŒ‹‰Ê‘Ò‚¿
		WaitMatch,

		// NAT Hole Punchˆ—’†
		Punch,

		// Ú‘±æŒˆ’èŠ®—¹
		Complete,

		// ƒGƒ‰[
		Error,
	};

public:

	/// <summary>
	/// ƒRƒ“ƒXƒgƒ‰ƒNƒ^
	/// </summary>
	/// <param name="mode">ƒzƒXƒg or ƒNƒ‰ƒCƒAƒ“ƒg</param>
	/// <param name="roomNumber">‚ ‚¢‚±‚Æ‚Î</param>
	/// <param name="enetHost">NetWorkManager‚ª¶¬‚µ‚½ENetHost</param>
	/// <param name="localPort">OS‚©‚ç©“®Š„‚è“–‚Ä‚³‚ê‚½ENet‚Ìƒ|[ƒg</param>
	HostAddressProvider(MODE mode, unsigned short roomNumber, ENetHost* enetHost, enet_uint16 localPort);
	~HostAddressProvider() = default;

	// XV
	void Update(void);
	// I—¹
	void End(void);

	/// <summary>
	/// iƒNƒ‰ƒCƒAƒ“ƒg—pjÅI“I‚ÉŒˆ’è‚µ‚½Ú‘±æ‚ğæ“¾
	/// </summary>
	/// <param name="endpoint"></param>
	/// <param name="route"></param>
	/// <returns>true = Œˆ’èÏ‚İAfalse = ‚Ü‚¾ŒŸõ’†</returns>
	bool GetConnectEndpoint(NetEndpoint& endpoint, CONNECTION_ROUTE& route) const;


	// ó‘ÔŠm”FiŠ®—¹j
	bool IsComplete(void) const { return state == STATE::Complete; }

	// ó‘ÔŠm”FiƒGƒ‰[j
	bool IsError(void) const { return state == STATE::Error; }

	// ENetƒQ[ƒ€ƒ|[ƒg
	enet_uint16 GetLocalPort(void) const { return localEndpoint.port; }

private:

#pragma region ƒ}ƒbƒ`ƒ“ƒOƒT[ƒo[

	// Oracle Cloud
	static constexpr const char* MATCH_SERVER_IP = "161.33.190.216";

	// Oracle‘¤‚Ì§Œä—pUDPƒ|[ƒg
	static constexpr int MATCH_SERVER_PORT = 50000;

#pragma endregion


#pragma region ƒ^ƒCƒ€ƒAƒEƒg“™

	// ƒT[ƒo[‚Ö‚ÌÄ‘—ŠÔŠu
	static constexpr int SERVER_SEND_COOLTIME = 60;

	// Hole Punch‘—MŠÔŠu
	static constexpr int PUNCH_SEND_COOLTIME = 10;

	// Hole PunchÅ‘å‘—M‰ñ”
	static constexpr int PUNCH_MAX_COUNT = 60;

#pragma endregion


private:

#pragma region Šî–{î•ñ

	// ƒ‚[ƒhiƒzƒXƒg or ƒNƒ‰ƒCƒAƒ“ƒgj
	MODE mode;

	// XVó‘Ô
	STATE state;

	// ‚ ‚¢‚±‚Æ‚Î
	unsigned short roomNumber;

	// ‚±‚ÌPC‚ğ¯•Ê‚·‚éID
	std::string machineId;

#pragma endregion

	// ENet
	// NetWorkManager‚ªŠ—LB
	// HostAddressProvider‘¤‚Å‚Ídelete‚µ‚È‚¢B
	ENetHost* enetHost;

	// Oracle§Œä—pƒ\ƒPƒbƒg
	// ENet‚Æ‚Í•ÊB
	// ƒ}ƒbƒ`ƒ“ƒOî•ñ‚Ì‚â‚èæ‚èê—pB
	int controlSocket;

	// ©•ª©g‚ÌÚ‘±î•ñ``````````````````

	// LAN“à
	NetEndpoint localEndpoint;

	// Oracle‚©‚çŠÏ‘ª‚³‚ê‚½ENet‘¤Public Endpoint
	NetEndpoint publicEndpoint;

	// ``````````````````©•ª©g‚ÌÚ‘±î•ñ


	// ƒzƒXƒgî•ñ````````````````````````

	std::string hostMachineId;

	NetEndpoint hostLocalEndpoint;

	NetEndpoint hostPublicEndpoint;

	// ````````````````````````ƒzƒXƒgî•ñ


	// ÅI“I‚ÉNetWorkManager‚Ö“n‚·Ú‘±æ````````````

	NetEndpoint connectEndpoint;

	CONNECTION_ROUTE connectionRoute;

	// ````````````ÅI“I‚ÉNetWorkManager‚Ö“n‚·Ú‘±æ


	// ƒJƒEƒ“ƒ^`````````````````````````

	int serverSendCounter;

	int punchSendCounter;

	int punchCount;

	// `````````````````````````ƒJƒEƒ“ƒ^

private:

	// ƒzƒXƒg—pXV
	void HostUpdate(void);

	// ƒNƒ‰ƒCƒAƒ“ƒg—pXV
	void ClientUpdate(void);


	// OracleŠÖ˜A````````````````````````

	// ©•ª©g‚ğOracle‚Ö“o˜^
	void RegisterToServer(void);

	// Oracle‚©‚ç‚ÌóM
	void ReceiveServerMessage(void);

	// ENet©g‚Ìƒ\ƒPƒbƒg‚©‚çOracle‚ÖƒpƒPƒbƒg‚ğ‘—‚èA
	// NATŒã‚ÌPublic IP / Port‚ğOracle‚ÉŠÏ‘ª‚³‚¹‚é
	void SendEndpointProbe(void);

	// ````````````````````````OracleŠÖ˜A


	// ‘Šè‚ÌPublic Endpoint‚ÖAENet©g‚Ìƒ\ƒPƒbƒg‚©‚çUDP‚ğ‘—M
	void SendPunchPacket(const NetEndpoint& endpoint);

	// Ú‘±æŒˆ’è
	void DecideConnectionRoute(void);

	// PC / LANî•ñ```````````````````````

	// PCŒÅ—LID‚Ìæ“¾E¶¬
	std::string GetOrCreateMachineId(void);

	// LAN“àIPv4æ“¾
	std::string GetLocalIPv4(void);

	// ```````````````````````PC / LANî•ñ


	// ƒ†[ƒeƒBƒŠƒeƒB``````````````````````

	bool SendControlMessage(const std::string& message);

	bool SendRawFromENet(const NetEndpoint& endpoint, const std::string& message);

	// ``````````````````````ƒ†[ƒeƒBƒŠƒeƒB
};