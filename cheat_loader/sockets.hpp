#pragma once
#include <WinSock2.h>
#include <string_view>

namespace sockets
{
	class tcp_client
	{
	public:
		explicit tcp_client(std::string_view host, std::string_view port);
		~tcp_client();

		template<typename T>
		std::size_t receive_raw(T &buf, bool should_reconnect = false, std::size_t size = 0)
		{
			auto size_specified = false;
			if (size == 0)
				size = buf.size();
			else
				size_specified = true;

			if (should_reconnect)
				reconnect();

			for (auto i = 0u; i < size;)
			{
				auto bytes_read = ::recv(_socket, reinterpret_cast<char*>(buf.data()) + i, size - i, 0);
				if (bytes_read > 0 || size_specified) // if size is specified then we wait until the buffer is filled
					i += bytes_read;
				else return i;
			}
			return size;
		}

		template<typename T>
		void send_raw(T &buf, bool should_reconnect = false, std::size_t size = 0)
		{
			if (size == 0)
				size = buf.size();

			if (should_reconnect)
				reconnect();

			for (auto i = 0u; i < size;)
				i += ::send(_socket, reinterpret_cast<const char*>(buf.data()) + i, size - i, 0);
		}
	private:
		SOCKET _socket;
		struct addrinfo *_result;

		void connect();
		void disconnect();
		void reconnect();
	};
}