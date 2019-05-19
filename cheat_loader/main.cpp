#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#include <memory>
#include "gui.hpp"
#include "xorstr.hpp"
#include "sockets.hpp"
#include "task_manager.hpp"
#include "authentication.hpp"
#include "program_binary.hpp"

namespace global
{
	std::unique_ptr<gui::menu> menu;
	std::unique_ptr<management::task_manager> task_manager;
}

void safe_exit()
{
	while (global::menu->get_running())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	exit(0);
}

void load(std::string_view username, std::string_view password)
{
	global::task_manager->inc_idx();

	global::task_manager->register_task<bool>(_xor_("checking for anticheat").c_str(), []()
	{
		void* drivers[1024] = { 0 };
		::EnumDeviceDrivers(drivers, sizeof(drivers), 0);

		for (int i = 0; drivers[i]; ++i)
		{
			char driver_name[MAX_PATH];
			::GetDeviceDriverBaseNameA(drivers[i], driver_name, MAX_PATH);

			if (!::_stricmp(driver_name, _xor_("BEDaisy.sys").c_str()) || !::_stricmp(driver_name, _xor_("EasyAntiCheat.sys").c_str()))
				global::task_manager->exception(_xor_("anticheat running").c_str());
		}
		return true;
	}, safe_exit);

	auto tcp_client = global::task_manager->register_task<std::shared_ptr<sockets::tcp_client>>(_xor_("creating tcp socket").c_str(), []()
	{
		return std::make_shared<sockets::tcp_client>(_xor_("curiosity.clever-code.net").c_str(), _xor_("8000").c_str());
		//return std::make_shared<sockets::tcp_client>(_xor_("192.168.2.2").c_str(), _xor_("8000").c_str());
	}, safe_exit, true);

	auto auth = std::make_unique<loader::authentication>(tcp_client, username, password);

	auto binary = global::task_manager->register_task<std::unique_ptr<memory::program_binary>>(_xor_("streaming binary").c_str(), [&auth]()
	{
		return auth->stream_binary();
	}, safe_exit, true);

	global::task_manager->register_task<bool>(_xor_("executing binary").c_str(), [&binary]()
	{
		binary->memory_execute();
		return true;
	}, safe_exit, true);

	global::menu->set_running(false);
}

int WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, int show_cmd)
{
	global::task_manager = std::make_unique<management::task_manager>(5);
	global::menu = std::make_unique<gui::menu>(_xor_("Curiosity Loader").c_str());

	while (global::menu->get_running())
		global::menu->render();
}