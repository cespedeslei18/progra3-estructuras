#pragma once


#include "datos/cargarDatos.h"
#include "datos/listaSimple.h"

#include <string>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

class Server;

typedef void(*MessageReceivedHandler)(Server* listener, int socketId, std::string msg);

class Server {
public:

	Server(std::string ipAddr, int port, MessageReceivedHandler handler);
	Server(string ipAddr, MessageReceivedHandler handler);

	~Server(){ Cleanup(); }

	void agregarArboles();

	// Metodos para los clientes del servidor
	bool login(std::string cedula, string tipo);
	std::string consultarPrecio(std::string codSuper,std::string codProducto);
	std::string consultarDescuento(std::string cedula);
	std::string consultarProductos(std::string codSuper);
	std::string comprar(string codSuper,string nombre, int cant, float precio);
	std::string verCarrito();
	void facturar();
	void descuentoExtra();
	std::string registrarse(std::string codCiudad, std::string cedula,std::string nombre, std::string telefono);

	// Metodos para el admin
	std::string insertarProducto(string codSuper,string codigo,string nombre,int cantidad,float precio);
	std::string modificarProducto(string codSuper, string codigo, string respuesta, string cambio);
	std::string consultarPrecioAdmin(string codSuper, string codigo);
	std::string consultarProductosAdmin(string codSuper);
	std::string consultarDescuentosAdmin(string cedula);
	std::string registrarCliente(string cedula);
	void reportes();
	void verDatos();
	std::string abrirServidor();

	void cargarDatos();
	void menuAdmin();

	// Metodos para levantar el servidor
	void Send(int clientSocket, std::string msg);
	bool Init();
	void Run();
	void Cleanup();

	void setPort(int p) { my_port = p; }

	int tipoUsuario = 4;
	string cedulaUser;
	lista colaUsuarios;

	// Arboles
	Binario ciudades;
	RyN supers;
	listaDC usuarios;
	ArbolB users;
	avlTree funcionarios;

private:
	SOCKET CreateSocket();
	SOCKET WaitForConnection(SOCKET listening);
	std::string my_ip;
	int my_port;
	MessageReceivedHandler message;

};