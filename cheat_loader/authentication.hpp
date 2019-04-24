#pragma once
#include <vector>
#include <string_view>
#include "sockets.hpp"
#include "program_binary.hpp"

namespace loader
{
	class authentication
	{
	public:
		authentication(std::shared_ptr<sockets::tcp_client> tcp_client, std::string_view username, std::string_view password);
		std::unique_ptr<memory::program_binary> stream_binary() const;
	private:
		std::size_t get_binary_size() const;
		std::shared_ptr< std::vector<char>> get_binary(std::string_view username, std::string_view password, std::size_t binary_size) const;

		std::shared_ptr<sockets::tcp_client> _tcp_client;
		std::string_view _username;
		std::string_view _password;
	};
}