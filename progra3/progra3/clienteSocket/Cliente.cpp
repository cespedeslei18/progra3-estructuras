#include "Cliente.h"
#include <string>
#include <iostream>
#include <WS2tcpip.h>
#pragma	comment (lib,"ws2_32.lib")

#define BUFFER_SIZE (4096)

using namespace std;

Cliente::Cliente(std::string ip, int port, MessageReceivedHandler handler) {
	serverIp = ip;
	serverPort = port;
	message = handler;
}

Cliente::~Cliente(){
	Cleanup();
}

void Cliente::Send(int serverSocket, std::string msg){
	send(serverSocket, msg.c_str(), msg.size() + 1, 0);
}

bool Cliente::Init(){
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsInit = WSAStartup(ver, &data);
	return wsInit == 0;
}

void Cliente::Run(){
	SOCKET sock = CreateSocket();
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIp.c_str(), &hint.sin_addr);
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR) {
		cerr << "No se pudo conectar al servidor" << endl;
		closesocket(sock);
		Cleanup();
		return;
	}

	// Ciclo principal
	char buf[4096];
	string input;
	// login
	cout << "Bienvenido al super" << endl;
	cout << endl;
	cout << "LOGIN" << endl;
	cout << "Tipo: " << endl;
	do {
		cout << "> ";
		// Obtener input de la consola
		getline(cin, input);
		// Enviar el texto
		int sendOk = send(sock, input.c_str(), input.size() + 1, 0);
		if (sendOk != SOCKET_ERROR) {
			ZeroMemory(buf, 4096);
			int bytesReceived = recv(sock, buf, 4096, 0);
			if (bytesReceived > 0) {
				cout << string(buf, 0, bytesReceived) << endl;
			}
		}

	} while (input.size() > 0);
	closesocket(sock);
	Cleanup();
}
void Cliente::Cleanup() { WSACleanup(); }

SOCKET Cliente::CreateSocket(){
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock != INVALID_SOCKET){
		return sock;
	}
	return -1;
}