#pragma once
#include <mutex>
#include <string_view>
#include <d3d11.h>
#include "imgui/imgui.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

namespace gui
{
	constexpr uint32_t wnd_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
	const ImVec2 wnd_size = { 220, 117 };

	class menu
	{
	public:
		explicit menu(std::string_view header);
		~menu();
		void render();
		bool get_running();
		void set_running(bool toggle);
	private:
		void draw_menu();

		bool create_device(HWND hwnd);
		void cleanup_device();
		void create_render_target();
		void cleanup_render_target();
		friend LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		std::mutex _m;
		bool _running;

		std::string_view _header;

		ID3D11Device* _p_device = nullptr;
		ID3D11DeviceContext* _p_device_context = nullptr;
		IDXGISwapChain* _p_swapchain = nullptr;
		ID3D11RenderTargetView* _p_render_target_view = nullptr;

		WNDCLASSEX _wc = { 0 };
		HWND _hwnd = { 0 };
		MSG _msg = { 0 };
	};
}