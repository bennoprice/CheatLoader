#include <array>
#include <vector>
#include <iostream>
#include "xorstr.hpp"
#include "task_manager.hpp"
#include "authentication.hpp"

namespace global
{
	extern std::unique_ptr<management::task_manager> task_manager;
}

namespace loader
{
	enum class requests
	{
		size = '1',
		binary = '2'
	};

	enum class errors
	{
		credentials = '1',
		hwid = '2'
	};

	authentication::authentication(std::shared_ptr<sockets::tcp_client> tcp_client, std::string_view username, std::string_view password)
		: _tcp_client(tcp_client)
		, _username(username)
		, _password(password)
	{ }

	std::unique_ptr<memory::program_binary> authentication::stream_binary() const
	{
		auto binary_size = get_binary_size();
		std::cout << _xor_("[+] got binary size: ") << binary_size << std::endl;

		auto binary = get_binary(_username, _password, binary_size);
		std::cout << _xor_("[+] streamed binary") << std::endl;

		return std::make_unique<memory::program_binary>(binary);
	}

	std::size_t authentication::get_binary_size() const
	{
		constexpr std::array<char, 1> size_request = { static_cast<char>(requests::size) };
		_tcp_client->send_raw(size_request);

		std::array<char, 10> size_response_buf = { 0 };
		if (_tcp_client->receive_raw(size_response_buf) == 0)
			global::task_manager->exception(_xor_("failed to connect to server").c_str()); // failed to get binary size

		return ::atoi(size_response_buf.data());
	}

	std::shared_ptr<std::vector<char>> authentication::get_binary(std::string_view username, std::string_view password, std::size_t binary_size) const
	{
		std::vector<char> binary_request(3);
		binary_request[0] = static_cast<char>(requests::binary);
		binary_request[1] = '&';
		binary_request.insert(binary_request.begin() + 2, username.begin(), username.end());
		binary_request[username.length() + 2] = '&';
		binary_request.insert(binary_request.begin() + username.length() + 3, password.begin(), password.end());

		std::vector<char> program_binary_buf(binary_size);
		_tcp_client->send_raw(binary_request, true);
		if (_tcp_client->receive_raw(program_binary_buf) != binary_size)
		{
			switch (static_cast<errors>(program_binary_buf[0]))
			{
			case errors::credentials:
				global::task_manager->exception(_xor_("incorrect credentials").c_str());
				break;
			case errors::hwid:
				global::task_manager->exception(_xor_("incorrect hwid").c_str());
				break;
			default:
				global::task_manager->exception(_xor_("failed to stream binary").c_str());
				break;
			}
		}
		return std::make_shared<std::vector<char>>(program_binary_buf);
	}
}