#include "Server.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

constexpr auto ADMIN = ("1.Insertar producto\n2.Modificar producto\n3.Consultar precio\n4.Consultar descuentos\n5.Consultar productos de un super\n6.Registrar clientes\n7.Reportes\n8.Ver datos\nQue desea hacer: ");
constexpr auto VENDEDOR = ("1.Consultar precio\n2.Consultar descuento de un cliente\n3.Consultar productos de un super\n4.Cerrar sesion\nQue desea hacer: ");
constexpr auto NOREGISTRADO = ("1.Consultar precio\n2.Consultar productos\n3.Registrarse\nQue desea hacer: ");
constexpr auto CLIENTE = ("1.Consultar precio\n2.Consultar descuento\n3.Consultar productos\n4.Comprar(carrito)\n5.Ver carrito\n6.Facturacion\n7.Cerrar sesion\nQue desea hacer: ");
constexpr auto FUNCIONARIO = ("1.Consultar precio\n2.Consultar descuento\n3.Consultar productos\n4.Comprar(carrito)\n5.Ver carrito\n6.Facturacion\n7.Descuento extra(5%)\n8.Cerrar sesion\nQue desea hacer: ");
constexpr auto LOGIN = ("Cedula: ");


#define BUFFER_SIZE (4096)

using namespace std;

void messageHandler(Server* listener, int client, string msg,int &tipoUsuario);

Server server("127.0.0.1",54010, messageHandler); //variable del Servidor

void messageHandler(Server* listener, int client, string msg,int &tipoUsuario) {
	char buf[BUFFER_SIZE];
	ZeroMemory(buf, BUFFER_SIZE);
	if (tipoUsuario == 4) { // Respuestas del login
		if (msg != "0" && msg != "1" && msg != "2" && msg != "3") {
			listener->Send(client, "Tipo invalido\nTipo:");
			return;
		}
		listener->Send(client, LOGIN);
		recv(client, buf, 4096, 0); // Recibir cedula del cliente
		string cedula = buf;
		if (server.login(cedula, msg)) {
			if (msg == "0") {
				listener->Send(client, CLIENTE);
				tipoUsuario = 0;
			}
			else if (msg == "1") {
				listener->Send(client, ADMIN);
				tipoUsuario = 1;
			}
			else if (msg == "2") {
				listener->Send(client, VENDEDOR);
				tipoUsuario = 2;
			}
			else if (msg == "3") {
				listener->Send(client, FUNCIONARIO);
				tipoUsuario = 3;
			}
			else {
				listener->Send(client, "Respuesta invalida");
			}
		}
		else if (msg == "0") {
			listener->Send(client, NOREGISTRADO);
			tipoUsuario = 5;
		}
		else
			listener->Send(client, "Usuario no encontrado\nTipo:");
	}
	else if (tipoUsuario == 0) { // Respuestas del cliente
		if (msg == "1") { // Consultar precio
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigos del cliente
			string codSuper = buf;
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			listener->Send(client, server.consultarPrecio(codSuper,codigo)+"\n\n"+CLIENTE); // Enviarle el precio
		}
		else if (msg == "2") { // consultarDescuento
			listener->Send(client,server.consultarDescuento(server.cedulaUser)); // Enviarle el descuento
		}
		else if (msg == "3") { // consultarProductos
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			listener->Send(client, "Productos: "+server.consultarProductos(codSuper) + "\n\n" + CLIENTE); // Enviarle la lista de productos
		}
		else if (msg == "4") { // comprar
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			if (server.supers.buscar(codSuper) == NULL) {
				listener->Send(client,"Super no encontrado");
				return;
			}
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			if (server.supers.buscar(codSuper)->inventario->buscar(codigo) == NULL) {
				listener->Send(client, "Producto no encotrado");
				return;
			}
			listener->Send(client, "Cantidad: ");
			recv(client, buf, 4096, 0);
			int cant = stoi(buf);
			NodoAA *prod = server.supers.buscar(codSuper)->inventario->buscar(codigo);
			if (prod->cantidad == 0) {
				listener->Send(client, "Ya no quedan");
				return;
			}
			while (prod->cantidad < cant) {
				listener->Send(client, "Solo hay " + to_string(prod->cantidad)+"\nCuantos quiere?");
				recv(client, buf, 4096, 0);
				cant = stoi(buf);
			}
			prod->cantidad -= cant;
			listener->Send(client, server.comprar(codSuper, prod->nombre, cant, prod->precioU,tipoUsuario) + "\n\n" + CLIENTE);
		}
		else if (msg == "5") { // verCarrito
			string carrito = server.verCarrito(tipoUsuario);
			if (carrito == "") {
				listener->Send(client, "El carrito esta vacio");
			}
			else {
				listener->Send(client, "Carrito: " + carrito + "\n\n" + CLIENTE);
			}
		}
		else if (msg == "6") { // facturar
			server.facturar(tipoUsuario);
			listener->Send(client, "Factura lista");
		}
		else if (msg == "7") { // Cerrar sesion
			listener->Send(client, "Tipo: ");
			tipoUsuario = 4;
		}
		else {
			listener->Send(client, "Respuesta invalida");
		}
	}
	else if (tipoUsuario == 1) { // ADMIN
		if (msg == "1") {
			string codSuper, cedula, nombre, cantidad, precio;
			listener->Send(client,"Codigo del super: ");
			recv(client, buf, 4096, 0);
			codSuper = buf;
			listener->Send(client,"Cedula: ");
			recv(client, buf, 4096, 0);
			cedula = buf;
			listener->Send(client,"Nombre: ");
			recv(client, buf, 4096, 0);
			nombre = buf;
			listener->Send(client,"Cantidad: ");
			recv(client, buf, 4096, 0);
			cantidad = buf;
			listener->Send(client,"Precio: ");
			recv(client, buf, 4096, 0);
			precio = buf;
			listener->Send(client, server.insertarProducto(codSuper, cedula, nombre, stoi(cantidad), stof(precio)));
		}
		else if (msg == "2") {
			string codSuper, codigo, respuesta, cambio;
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0);
			codSuper = buf;
			listener->Send(client, "Codigo: ");
			recv(client, buf, 4096, 0);
			codigo = buf;
			listener->Send(client, "Que desea cambiar:\n1.Codigo de super\n2.Codigo\n3.Nombre\n4.Cantidad\n5.Precio\n");
			recv(client, buf, 4096, 0);
			respuesta = buf;
			listener->Send(client, "Cambio: ");
			recv(client, buf, 4096, 0);
			cambio = buf;
			listener->Send(client, server.modificarProducto(codSuper, codigo, respuesta, cambio));
		}
		else if (msg == "3") {
			string codSuper, codigo;
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0);
			codSuper = buf;
			listener->Send(client, "Cedula: ");
			recv(client, buf, 4096, 0);
			codigo = buf;
			listener->Send(client, server.consultarPrecioAdmin(codSuper, codigo));
		}
		else if (msg == "4") {
			string cedula;
			listener->Send(client, "Cedula: ");
			recv(client, buf, 4096, 0);
			cedula = buf;
			listener->Send(client, server.consultarDescuentosAdmin(cedula));
		}
		else if (msg == "5") {
			string codSuper;
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0);
			codSuper = buf;
			listener->Send(client, server.consultarProductosAdmin(codSuper));
		}
		else if (msg == "6") {
			if (server.colaUsuarios.ListaVacia()) {
				listener->Send(client, "No hay usuarios en la cola de espera");
			}
			else {
				listener->Send(client, server.colaUsuarios.toString());
				string cedula;
				listener->Send(client, "Cual usuario desea registrar (cedula): ");
				recv(client, buf, 4096, 0);
				cedula = buf;
				while (!server.colaUsuarios.buscar(cedula)) {
					listener->Send(client, "Cedula no encontrada\nCual usuario desea registrar (cedula): ");
					recv(client, buf, 4096, 0);
					cedula = buf;
				}
				listener->Send(client, server.registrarCliente(cedula) + "\n" + ADMIN);
			}
		}
		else if (msg == "7") {
			// reportes
		}
		else if (msg == "8") {
			server.verDatos();
		}
		else if (msg == "9") { // Cerrar sesion
			listener->Send(client, "Tipo: ");
			tipoUsuario = 4;
		}
		else{
			listener->Send(client,"Respuesta invalida");
		}
	}
	else if (tipoUsuario == 2) { // Respuestas del vendedor
		if (msg == "1") { // Consultar precio
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigos del cliente
			string codSuper = buf;
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			listener->Send(client, server.consultarPrecio(codSuper, codigo) + "\n\n" + VENDEDOR); // Enviarle el precio
		}
		else if (msg == "2") {// consultarDescuento
			listener->Send(client, "Cedula: ");
			recv(client, buf, 4096, 0); // Recibir cedula del cliente
			string cedula = buf;
			listener->Send(client, server.consultarDescuento(cedula) + "\n\n" + VENDEDOR); // Enviarle el descuento
		}
		else if (msg == "3") { // Consultar productos
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			listener->Send(client, "Productos: " + server.consultarProductos(codSuper) + "\n\n" + VENDEDOR); // Enviarle la lista de productos
		}
		else if (msg == "4") {
			listener->Send(client, "Tipo: ");
			tipoUsuario = 4;
		}
		else {
			listener->Send(client, "Respuesta invalida");
		}
	}
	else if (tipoUsuario == 5) { // Respuestas del cliente no registrado
		if (msg == "1") {// Consultar precio
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigos del cliente
			string codSuper = buf;
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			listener->Send(client, server.consultarPrecio(codSuper, codigo) + "\n\n" + NOREGISTRADO); // Enviarle el precio
			}
		else if (msg == "2") { // Consultar productos
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			listener->Send(client, "Productos: " + server.consultarProductos(codSuper) + "\n\n" + NOREGISTRADO); // Enviarle la lista de productos
		}
		else if (msg == "3") { //registrarse
			listener->Send(client, "Codigo de ciudad: ");
			recv(client, buf, 4096, 0); 
			string codCiudad = buf;
			listener->Send(client, "Cedula: ");
			recv(client, buf, 4096, 0);
			string cedula = buf;
			listener->Send(client, "Nombre: ");
			recv(client, buf, 4096, 0);
			string nombre = buf;
			listener->Send(client, "Telefono: ");
			recv(client, buf, 4096, 0);
			string telefono = buf;
			listener->Send(client, server.registrarse(codCiudad,cedula,nombre,telefono) + "\n\n" + NOREGISTRADO); 
		}
		else {
			listener->Send(client, "Respuesta invalida");
		}
	}
	else {  // Respuestas del funcionario
		if (msg == "1") { // Consultar precio
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigos del cliente
			string codSuper = buf;
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			listener->Send(client, server.consultarPrecio(codSuper, codigo) + "\n\n" + FUNCIONARIO); // Enviarle el precio
		}
		else if (msg == "2") { // consultarDescuento
			listener->Send(client, server.consultarDescuento(server.cedulaUser) + "\n\n" + FUNCIONARIO); // Enviarle el descuento
		}
		else if (msg == "3") { // Consultar productos
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			listener->Send(client, server.consultarProductos(codSuper) + "\n\n" + FUNCIONARIO); // Enviarle la lista de productos
		}
		else if (msg == "4") { // comprar
			listener->Send(client, "Codigo del super: ");
			recv(client, buf, 4096, 0); // Recibir codigo del cliente
			string codSuper = buf;
			if (server.supers.buscar(codSuper) == NULL) {
				listener->Send(client, "Super no encontrado");
				return;
			}
			listener->Send(client, "Codigo del producto: ");
			recv(client, buf, 4096, 0);
			string codigo = buf;
			if (server.supers.buscar(codSuper)->inventario->buscar(codigo) == NULL) {
				listener->Send(client, "Producto no encotrado");
				return;
			}
			listener->Send(client, "Cantidad: ");
			recv(client, buf, 4096, 0);
			int cant = stoi(buf);
			NodoAA *prod = server.supers.buscar(codSuper)->inventario->buscar(codigo);
			if (prod->cantidad == 0) {
				listener->Send(client, "Ya no quedan");
				return;
			}
			while (prod->cantidad < cant) {
				listener->Send(client, "Solo hay " + to_string(prod->cantidad) + "\nCuantos quiere?");
				recv(client, buf, 4096, 0);
				cant = stoi(buf);
			}
			prod->cantidad -= cant;
			listener->Send(client, server.comprar(codSuper, prod->nombre, cant, prod->precioU,tipoUsuario) + "\n\n" + FUNCIONARIO);
		}
		else if (msg == "5") { // verCarrito
			string carrito = server.verCarrito(tipoUsuario);
			if (carrito == "") {
				listener->Send(client, "El carrito esta vacio");
			}
			else {
				listener->Send(client, "Carrito: "+carrito + "\n\n" + FUNCIONARIO);
			}
		}
		else if (msg == "6") { // facturar
			server.facturar(tipoUsuario);
			listener->Send(client, "Factura lista");
		}
		else if (msg == "7") { // descuentoExtra
			server.descuentoExtra();
			listener->Send(client, "Descuento extra aplicado");
		}
		else if (msg == "8") { // Cerrar sesion
			listener->Send(client, "Tipo: ");
			tipoUsuario = 4;
		}
		else {
			listener->Send(client, "Respuesta invalida");
		}
	}
}

Server::Server(std::string ipAddr, int port, MessageReceivedHandler handler) {
	my_ip = ipAddr;
	my_port = port;
	message = handler;
}

Server::Server(string ipAddr, MessageReceivedHandler handler) {
	my_ip = ipAddr;
	message = handler;
}

void Server::agregarArboles() {
	for (int i = 0; i < 4; i++) {
		usuarios.InsertarFinal(to_string(i));
	}
	usuarios.agregarArbol(&users,&funcionarios);
}

// Enviar mensaje a un cliente
void Server::Send(int clientSocket, std::string msg) {
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}
// Inicializar winsock
bool Server::Init() {
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	// TODO: agarrar error

	return wsInit == 0;
}
// Ciclo principal
void Server::Run() {
	char buf[BUFFER_SIZE];
	string input;
	int tipoUsuario = 4;
	//while (true) {
		SOCKET listening = CreateSocket();
		if (listening == INVALID_SOCKET) {
			//break;
			return;
		}

		SOCKET client = WaitForConnection(listening);
		if (client != INVALID_SOCKET) {
			closesocket(listening);

			int bytesReceived = 0;
			do {
				ZeroMemory(buf, BUFFER_SIZE);

				bytesReceived = recv(client, buf, 4096, 0);
				if (bytesReceived > 0) {
					if (message != NULL) {
						cout << "Antes: " << tipoUsuario << endl;
						message(this, client, std::string(buf, 0, bytesReceived),tipoUsuario);
						cout << "Despues: " << tipoUsuario << endl;
						//cout << "CLIENT> " << string(buf, 0, bytesReceived) << endl;
					}
				}
			} while (bytesReceived > 0);
			closesocket(client);
			tipoUsuario = 4;
			cout << "Cliente desconectado" << endl;
		}
	//}
}
// Limpiar winsock
void Server::Cleanup() {
	WSACleanup();
}

SOCKET Server::CreateSocket() {
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening != INVALID_SOCKET) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(my_port);
		inet_pton(AF_INET, my_ip.c_str(), &hint.sin_addr);

		int bindOk = ::bind(listening, (sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR) {
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR) {
				return -1;
			}
		}
		else {
			return -1;
		}
	}
	return listening;
}

SOCKET Server::WaitForConnection(SOCKET listening) {
	sockaddr_in client;
	int clientSize = sizeof(client);
	cout << "Esperando cliente..." << endl;
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "No se pudo conectar el cliente" << endl;
	}

	char host[NI_MAXHOST];    // Nombre del cliente
	char service[NI_MAXSERV]; // Puerto del cliente

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " conectado en el puerto " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " conectado en el puerto " << ntohs(client.sin_port) << endl;
	}
	return clientSocket;
}

// metodos de los clientes
string Server::consultarPrecio(string codSuper,string codProducto) {
	if (supers.buscar(codSuper) == NULL) {
		return "Super no encontrado";
	}
	else if (supers.buscar(codSuper)->inventario->buscar(codProducto) == NULL) {
		return "Producto no encontrado";
	}
	else {
		NodoAA *prod = supers.buscar(codSuper)->inventario->buscar(codProducto);
		return "Precio de " + prod->nombre + ": " + to_string(prod->precioU);
	}
}

string Server::consultarProductos(string codSuper) {
	if (supers.buscar(codSuper) == NULL) {
		return "Super no encontrado";
	}
	else {
		string msg="";
		supers.buscar(codSuper)->inventario->toString(supers.buscar(codSuper)->inventario->raiz,msg);
		return "Productos: " + msg;

	}
}

string Server::consultarDescuento(string cedula) {
	int n = 0;
	Pagina *p = users.buscar(cedula, n);
	if (p != NULL) {
		nodoUsuario *user = p->obtenerClave(n);
		return "Descuento de " + user->nombre + ": " + to_string(user->descuento) + "%";
	}
	else {
		avl_node *user = funcionarios.obtenerUsuario(cedula);
		if (user != NULL) {
			return "Descuento de " + user->nombre + ": " + to_string(user->descuento) + "%";
		}
		else {
			return "Cliente no encontrado";
		}
	}
}

void Server::descuentoExtra() {
	avl_node *user = funcionarios.obtenerUsuario(server.cedulaUser);
	user->descuento += 5;
}

string Server::comprar(string codSuper,string nombre,int cant,float precio, int tipoUsuario) {
	if (tipoUsuario == 0) {
		int n;
		Pagina *p = users.buscar(server.cedulaUser, n);
		nodoUsuario *user = p->obtenerClave(n);
		if (user->carrito.codSuper == "")
			user->carrito.setCodSuper(codSuper);
		else if (user->carrito.codSuper != codSuper)
			return "Ese producto esta en otro super al que esta comprando";
		user->carrito.InsertarFinal(nombre, cant, precio);
		return "Compra realizada con exito";
	}
	else {
		avl_node *user = funcionarios.obtenerUsuario(server.cedulaUser);
		user->carrito.InsertarFinal(nombre, cant, precio);
		return "Compra realizada con exito";
	}
}

string Server::verCarrito(int tipoUsuario) {
	if (tipoUsuario == 0) {
		int n;
		Pagina *p = users.buscar(server.cedulaUser, n);
		nodoUsuario *user = p->obtenerClave(n);
		return user->carrito.Mostrar();
	}
	else {
		avl_node *user = funcionarios.obtenerUsuario(server.cedulaUser);
		return user->carrito.Mostrar();
	}
}

void Server::facturar(int tipoUsuario) {
	if (tipoUsuario == 0) {
		int n;
		Pagina *p = users.buscar(server.cedulaUser, n);
		nodoUsuario *user = p->obtenerClave(n);
		user->facturas++;
		int nFacturas = user->facturas;
		int suma = 0;
		while (nFacturas != 0) {
			suma += (nFacturas % 10);
			nFacturas /= 10;
		}
		if (suma != 0 && suma % 3 == 0) {
			user->descuento += 1;
		}
		user->carrito.facturar(user->cedula, user->telefono, user->facturas, user->descuento, user->nombre);
		listaC nCarrito;
		user->carrito = nCarrito;
	}
	else {
		avl_node *user = funcionarios.obtenerUsuario(server.cedulaUser);
		user->facturas++;
		int nFacturas = user->facturas;
		int suma = 0;
		while (nFacturas != 0) {
			suma += (nFacturas % 10);
			nFacturas /= 10;
		}
		if (suma != 0 && suma % 3 == 0) {
			user->descuento += 1;
		}
		user->carrito.facturar(user->cedula, user->telefono, user->facturas, user->descuento, user->nombre);
		listaC nCarrito;
		user->carrito = nCarrito;
	}
}

string Server::registrarse(string codCiudad,string cedula,string nombre,string telefono) {
	if (server.ciudades.buscar(codCiudad) ==  NULL) {
		return "Ciudad no encontrada";
	}
	int n;
	if (server.users.buscar(cedula, n) != NULL) {
		return "Ya existe un cliente con esa cedula";
	}
	else if (server.colaUsuarios.buscar(cedula)) {
		return "Ya hay un usuario con esa cedula a la espera";
	}
	else {
		server.colaUsuarios.InsertarFinal(codCiudad, cedula, nombre, telefono, "0");
		return "Usuario añadido a la cola de espera";
	}
}


// metodos del admin
void Server::cargarDatos() {
	server.agregarArboles();
	ciudades = leerCiudades("datos/archivos_txt/Ciudad.txt");
	leerArchivo(supers, ciudades, "datos/archivos_txt/supermercado.txt", 0);
	leerArchivo(supers, ciudades, "datos/archivos_txt/Inventariodatos.txt", 1);
	leerUsuarios("datos/archivos_txt/Usuarios.txt", &funcionarios, &users);
	cout << endl;
}

string Server::insertarProducto(string codSuper, string codigo, string nombre, int cantidad, float precio) {
	if ((supers.buscar(codSuper) == NULL)) {
		return "Super no encontrado";
	}
	if (((supers.buscar(codSuper)->inventario->buscar(codigo))) != NULL) {
		return "Ya existe un producto con ese codigo";
	}
	supers.insertarProducto(codSuper, codigo, nombre, cantidad,precio);
	return "Insercion realizada con exito";
}

string Server::modificarProducto(string codSuper,string codigo,string respuesta,string cambio) {
	if ((supers.buscar(codSuper) == NULL)) {
		return "Super no encontrado";
	}
	if (((supers.buscar(codSuper)->inventario->buscar(codigo))) != NULL) {
		NodoAA *producto = (supers.buscar(codSuper)->inventario->buscar(codigo));
		if (respuesta == "1" && (supers.buscar(cambio) == NULL)) {
			return "Super no encontrado";
		}
		if (respuesta == "2" && (supers.buscar(codSuper)->inventario->buscar(cambio) != NULL)) {
			return "Ya existe un producto con ese codigo";
		}
		switch (stoi(respuesta)) {
		case 1:
			producto->codSuper = cambio;
			break;
		case 2:
			producto->codProducto = cambio;
			break;
		case 3:
			producto->nombre = cambio;
			break;
		case 4:
			producto->cantidad = stoi(cambio);
			break;
		case 5:
			producto->precioU = stof(cambio);
			break;
		default:
			return "Respuesta invalida";
		}
		return "Producto modificado con exito";
	}
	else {
		return "Producto no encontrado";
	}
}

string Server::consultarPrecioAdmin(string codSuper,string codigo) {
	if (supers.buscar(codSuper) == NULL) {
		return "Super no encontrado";
	}
	if (supers.buscar(codSuper)->inventario->buscar(codigo) == NULL) {
		return "Producto no encontrado";
	}
	else {
		NodoAA *prod = supers.buscar(codSuper)->inventario->buscar(codigo);
		return "Precio de " + prod->nombre + ": " + to_string(prod->precioU);
	}
}

string Server::consultarProductosAdmin(string codSuper) {
	if (supers.buscar(codSuper) == NULL) {
		return "Super no encontrado";
	}
	else {
		string msg;
		supers.buscar(codSuper)->inventario->toString(supers.buscar(codSuper)->inventario->raiz,msg);
		return "Productos: " + msg;
	}
}

void Server::verDatos() { //incompleta
	cout << "CIUDADES: ";
	ciudades.PostordenI();
	cout << endl;
	cout << endl;
	cout << "SUPERS (con sus inventarios): ";
	supers.mostrar(supers.primeroRN);
	cout << "USUARIOS: ";
	users.inOrden(users.raiz);
	funcionarios.inorder(funcionarios.root);
	cout << endl;
}

bool Server::login(string cedula, string tipo) {
	if (tipo == "3") {
		avl_node *user = funcionarios.obtenerUsuario(cedula);
		if (user != NULL && user->cedula == cedula) {
			server.cedulaUser = user->cedula;
			return true;
		}
		return false;
	}
	else {
		int n = 0;
		Pagina *p = users.buscar(cedula, n);
		if (p != NULL) {
			nodoUsuario *user = p->obtenerClave(n);
			if (user->cedula == cedula && user->tipo == tipo) {
				server.cedulaUser = user->cedula;
				return true;
			}
		}
		return false;
	}
}

string Server::consultarDescuentosAdmin(string cedula) {
	int n = 0;
	Pagina *p = users.buscar(cedula, n);
	if (p != NULL) {
		nodoUsuario *user = p->obtenerClave(n);
		return "Descuento de " + user->nombre + ": " + to_string(user->descuento) + "%" ;
	}
	else {
		avl_node *user = funcionarios.obtenerUsuario(cedula);
		if (user != NULL) {
			return "Descuento de " + user->nombre + ": " + to_string(user->descuento) + "%";
		}
		else {
			return "Cliente no encontrado";
		}
	}
}

string Server::registrarCliente(string cedula) {
		pnodoL usuario = server.colaUsuarios.buscarNodo(cedula);
		nodoUsuario nuevo(usuario->codCiudad, usuario->cedula, usuario->nombre, usuario->telefono, "0");
		server.users.insertar(nuevo);

		server.colaUsuarios.borrarPosicion(server.colaUsuarios.buscarPosicion(cedula));
		return "Cliente registrado con exito";
}

void abrirServer() {
	while(true)
		server.Run();
}

int main() {
	server.cargarDatos();
	server.Init();
	thread s1(abrirServer);
	thread s2(abrirServer);
	thread s3(abrirServer);
	thread s4(abrirServer);


	cin.get();
	cin.get();
	return 0;
}
