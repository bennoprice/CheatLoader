#include <iostream>
#include <WS2tcpip.h>
#include "gui.hpp"
#include "xorstr.hpp"
#include "sockets.hpp"
#pragma comment (lib, "ws2_32.lib")

namespace sockets
{
	tcp_client::tcp_client(std::string_view host, std::string_view port)
	{
		WSADATA wsa_data = { 0 };
		addrinfo hints = { 0 };

		::WSAStartup(MAKEWORD(2, 2), &wsa_data);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		::getaddrinfo(host.data(), port.data(), &hints, &_result);
		connect();
	}

	tcp_client::~tcp_client()
	{
		disconnect();
		::freeaddrinfo(_result);
		::WSACleanup();
	}

	void tcp_client::connect()
	{
		for (auto ptr = _result; ptr != nullptr; ptr = ptr->ai_next)
		{
			_socket = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			::connect(_socket, ptr->ai_addr, ptr->ai_addrlen);
		}
	}

	void tcp_client::disconnect()
	{
		::closesocket(_socket);
	}

	void tcp_client::reconnect()
	{
		disconnect();
		connect();
	}
}