#include <array>
#include <thread>
#include <iostream>
#include "gui.hpp"
#include "xorstr.hpp"
#include "task_manager.hpp"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#pragma comment(lib, "d3d11.lib")

extern void load(std::string_view username, std::string_view password);

namespace global
{
	extern std::unique_ptr<management::task_manager> task_manager;
	extern std::unique_ptr<gui::menu> menu;
}

namespace ImGui
{
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

namespace gui
{
	LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return true;

		switch (msg)
		{
		case WM_SYSCOMMAND:
			if ((wparam & 0xfff0) == SC_KEYMENU)
				return 0;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	menu::menu(std::string_view header)
		: _header(header)
	{
		// calc window pos
		RECT screen;
		GetWindowRect(GetDesktopWindow(), &screen);
		ImVec2 wnd_pos = { (screen.right / 2.f) - (wnd_size.x / 2.f), (screen.bottom / 2.f) - (wnd_size.y / 2.f) };

		// create window
		_wc = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0, 0, GetModuleHandle(NULL), 0, 0, 0, 0, L"window", 0 };
		RegisterClassEx(&_wc);
		_hwnd = CreateWindow(_wc.lpszClassName, 0, WS_POPUP, wnd_pos.x, wnd_pos.y, wnd_size.x, wnd_size.y, 0, 0, _wc.hInstance, 0);

		// init direct3d
		if (!create_device(_hwnd))
		{
			std::cout << _xor_("[-] failed to create device") << std::endl;
			cleanup_device();
			UnregisterClass(_wc.lpszClassName, _wc.hInstance);
			return;
		}

		// show window
		ShowWindow(_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(_hwnd);

		// setup imgui
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = 0;

		style.WindowRounding = 0.f;
		style.WindowPadding = { 10.f, 11.f };
		style.ItemSpacing = { 8.f, 6.f };
		//style.FrameBorderSize = 1.f;

		ImGui_ImplWin32_Init(_hwnd);
		ImGui_ImplDX11_Init(_p_device, _p_device_context);

		std::cout << _xor_("[+] initialised menu") << std::endl;
		set_running(true);
	}

	menu::~menu()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		cleanup_device();
		DestroyWindow(_hwnd);
		UnregisterClass(_wc.lpszClassName, _wc.hInstance);
	}

	void menu::set_running(bool toggle)
	{
		std::lock_guard<std::mutex> guard(_m);
		_running = toggle;
	}

	bool menu::get_running()
	{
		std::lock_guard<std::mutex> guard(_m);
		return _running;
	}

	void menu::render()
	{
		if (_msg.message == WM_QUIT)
		{
			set_running(false);
			return;
		}

		if (PeekMessage(&_msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&_msg);
			DispatchMessage(&_msg);
			return;
		}

		// start imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// draw menu
		static auto open = true;
		ImGui::SetNextWindowSize(wnd_size);
		ImGui::Begin(_header.data(), &open, wnd_flags);
		ImGui::SetWindowPos({ 0, 0 });
		draw_menu();
		ImGui::End();

		if (!open)
			set_running(false);

		// render
		ImGui::Render();
		_p_device_context->OMSetRenderTargets(1, &_p_render_target_view, 0);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		_p_swapchain->Present(1, 0);
	}

	void menu::draw_menu()
	{
		if (global::task_manager->get_progress() == 0.f)
		{
			static char username[32] = { 0 };
			static char password[32] = { 0 };
			static bool remember = true;

			ImGui::PushItemWidth(-1);
			ImGui::InputTextWithHint(_xor_("##username").c_str(), _xor_("username").c_str(), username, sizeof(username));
			ImGui::InputTextWithHint(_xor_("##password").c_str(), _xor_("password").c_str(), password, sizeof(password), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);

			ImGui::Spacing();

			if (ImGui::Button(_xor_("launch").c_str(), { 110, 0 }))
			{
				std::thread load_thread(load, username, password);
				load_thread.detach();
			}

			ImGui::SameLine();
			ImGui::Checkbox(_xor_("remember").c_str(), &remember);

			ImGui::PopItemWidth();
		}
		else
		{
			ImGui::PushItemWidth(-1);

			ImGui::SetCursorPosY(45.f);
			ImGui::ProgressBar(global::task_manager->get_progress());

			auto task = global::task_manager->get_task();
			ImGui::SetCursorPosY(70.f);
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() / 2.f) - (ImGui::CalcTextSize(task.status.data()).x / 2.f));
			task.error ? ImGui::TextColored({ 255.f, 0.f, 0.f, 255.f }, task.status.data()) : ImGui::Text(task.status.data());

			ImGui::PopItemWidth();
		}
	}

	bool menu::create_device(HWND hwnd)
	{
		// setup swapchain
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT create_device_flags = 0;
		D3D_FEATURE_LEVEL feature_level;
		const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		if (D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &_p_swapchain, &_p_device, &feature_level, &_p_device_context) != S_OK)
			return false;

		create_render_target();
		return true;
	}

	void menu::cleanup_device()
	{
		cleanup_render_target();
		if (_p_swapchain) { _p_swapchain->Release(); _p_swapchain = 0; }
		if (_p_device_context) { _p_device_context->Release(); _p_device_context = 0; }
		if (_p_device) { _p_device->Release(); _p_device = 0; }
	}

	void menu::create_render_target()
	{
		ID3D11Texture2D* p_back_buffer;
		_p_swapchain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));
		_p_device->CreateRenderTargetView(p_back_buffer, 0, &_p_render_target_view);
		p_back_buffer->Release();
	}

	void menu::cleanup_render_target()
	{
		if (_p_render_target_view) { _p_render_target_view->Release(); _p_render_target_view = 0; }
	}
}