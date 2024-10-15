#pragma once

#include <Windows.h>
#include <MinHook.h>
#include <dx11.hpp>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <var.hpp>
#include <menu.hpp>

typedef HRESULT(APIENTRY* IDXGISwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
IDXGISwapChainPresent oIDXGISwapChainPresent;

typedef void(APIENTRY* ID3D11DrawIndexed)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
ID3D11DrawIndexed oID3D11DrawIndexed;

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}

namespace DirectX11Interface {
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11RenderTargetView* RenderTargetView;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks {
	void Destroy() {
		MH_DisableHook(MH_ALL_HOOKS);
		DisableAll();
		MH_Uninitialize();
	}

	LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (var::showMenu) {
			if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) // adding an if here fixes the ui not responding to clicks etc.
				return true;
		}
		return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
	}

	HRESULT APIENTRY MJPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
		if (!var::imguiInit) {
			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&DirectX11Interface::Device))) {
				ImGui::CreateContext();

				ImGuiIO& io = ImGui::GetIO(); (void)io;
				ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
				io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

				DirectX11Interface::Device->GetImmediateContext(&DirectX11Interface::DeviceContext);

				DXGI_SWAP_CHAIN_DESC Desc;
				pSwapChain->GetDesc(&Desc);
				WindowHwnd = Desc.OutputWindow;

				ID3D11Texture2D* BackBuffer;
				pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
				DirectX11Interface::Device->CreateRenderTargetView(BackBuffer, NULL, &DirectX11Interface::RenderTargetView);
				BackBuffer->Release();

				ImGui_ImplWin32_Init(WindowHwnd);
				ImGui_ImplDX11_Init(DirectX11Interface::Device, DirectX11Interface::DeviceContext);
				Menu::MenuIO();
				Menu::StyleMenu();
				ImGui_ImplDX11_CreateDeviceObjects();
				ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
				Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
				var::imguiInit = true;
			}
		}
		if (GetAsyncKeyState(VK_INSERT) & 1) var::showMenu = !var::showMenu;
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::GetIO().MouseDrawCursor = var::showMenu;
		if (var::showMenu == true) {
			Menu::Render();
		}
		ImGui::EndFrame();
		ImGui::Render();
		DirectX11Interface::DeviceContext->OMSetRenderTargets(1, &DirectX11Interface::RenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		return oIDXGISwapChainPresent(pSwapChain, SyncInterval, Flags);
	}

	void APIENTRY MJDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) {

	}

	DWORD WINAPI MainThread(LPVOID lpParameter) {
		bool WindowFocus = false;
		while (WindowFocus == false) {
			DWORD ForegroundWindowProcessID;
			GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
			if (GetCurrentProcessId() == ForegroundWindowProcessID) {

				Process::ID = GetCurrentProcessId();
				Process::Handle = GetCurrentProcess();
				Process::Hwnd = GetForegroundWindow();

				RECT TempRect;
				GetWindowRect(Process::Hwnd, &TempRect);
				Process::WindowWidth = TempRect.right - TempRect.left;
				Process::WindowHeight = TempRect.bottom - TempRect.top;

				char TempTitle[MAX_PATH];
				GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
				Process::Title = TempTitle;

				char TempClassName[MAX_PATH];
				GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
				Process::ClassName = TempClassName;

				char TempPath[MAX_PATH];
				GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
				Process::Path = TempPath;

				WindowFocus = true;
			}
		}
		bool InitHook = false;
		while (InitHook == false) {
			if (DirectX11::Init() == true) {
				CreateHook(8, (void**)&oIDXGISwapChainPresent, MJPresent);
				CreateHook(12, (void**)&oID3D11DrawIndexed, MJDrawIndexed);
				InitHook = true;
			}
		}
		return 0;
	}
}
