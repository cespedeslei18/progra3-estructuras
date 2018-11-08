#include <iostream>
#include <string>
#include "Cliente.h"

using namespace std;

void Listener_MessageReceived(Cliente* listener, int client, string msg) {
	listener->Send(client, msg);
}

int main(){
	Cliente cliente("127.0.0.1", 54000, Listener_MessageReceived);
	if (cliente.Init()) {
		cliente.Run();
	}
	cin.get();
	return 0;
}