#pragma once

#include <string>
#include <WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

class Cliente;

typedef void(*MessageReceivedHandler)(Cliente* client, int socketId, std::string msg);

class Cliente {
public:
	Cliente(std::string serverIp, int serverPort, MessageReceivedHandler handler);

	~Cliente();

	void Send(int serverSocket, std::string msg);
	bool Init();
	void Run();
	void Cleanup();

private:
	SOCKET CreateSocket();
	SOCKET Connect();
	std::string serverIp;
	int serverPort;
	MessageReceivedHandler message;
};