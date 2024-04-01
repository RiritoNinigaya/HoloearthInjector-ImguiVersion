#pragma once
#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include "imgui/imstb_rectpack.h"
#include "imgui/imstb_textedit.h"
#include "imgui/imstb_truetype.h"
#include <d3d11.h>
#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Process/Process.h>
#include <BlackBone/Patterns/PatternSearch.h>
#include <BlackBone/Process/RPC/RemoteFunction.hpp>
#include <BlackBone/Syscalls/Syscall.h>
using namespace std;
using namespace blackbone;
namespace fs = std::filesystem;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// D3D11 Initialization For Device(As CPU or GPU) :D
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
#include "includes.h"
std::set<std::wstring> nativeMods, modList;

void DllLoadLibrary(Process& proc, const wstring& path) {
	auto mainThread = proc.threads().getMain();
	if (auto pLoadLibrary = MakeRemoteFunction<decltype(&LoadLibraryW)>(proc, L"Kernel32.dll", "LoadLibraryW"); pLoadLibrary && mainThread)
	{
		auto result = pLoadLibrary.Call({ path.c_str() }, mainThread);
		if (*result)
			cout << "LoadLibrary result: " << *result << "\n";
		else
		{
			cout << "Failed!!!" << endl;
			Sleep(3400);
			exit(554);
		}
	}
}

void DllManualMap(Process& proc, const wstring& path)
{

	nativeMods.clear();
	modList.clear();

	nativeMods.emplace(L"combase.dll");
	nativeMods.emplace(L"user32.dll");
	modList.emplace(L"windows.storage.dll");
	modList.emplace(L"shell32.dll");
	modList.emplace(L"shlwapi.dll");

	auto callback = [](CallbackType type, void* /*context*/, Process& /*process*/, const ModuleData& modInfo)
		{
			if (type == PreCallback)
			{
				if (nativeMods.count(modInfo.name))
					return LoadData(MT_Native, Ldr_None);
			}
			else
			{
				if (modList.count(modInfo.name))
					return LoadData(MT_Default, Ldr_ModList);
			}

			return LoadData(MT_Default, Ldr_None);
		};
	auto image = proc.mmap().MapImage(path, NoFlags, callback);
	if (!image)
		std::wcout << L"Mapping failed with error 0x" << std::hex << image.status
		<< L". " << Utils::GetErrorDescription(image.status) << std::endl << std::endl;
	else
		std::wcout << L"Injected!!!\n";
}