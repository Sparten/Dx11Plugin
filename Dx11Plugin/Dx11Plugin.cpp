//install directx SDK https://www.microsoft.com/en-us/download/details.aspx?id=6812
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <fstream>
#include "Dx11Plugin.hpp"
#include "InternalsPlugin.hpp"
#include "..\PolyHook\PolyHook.hpp"
#include "..\FontRendering\FW1FontWrapper.h"
#pragma comment(lib, "d3d11.lib")

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pDeviceContextVTable = NULL;

D3D11PresentHook phookD3D11Present = NULL;

IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;

bool firstTime = true;

// plugin information
extern "C" __declspec(dllexport)
const char * __cdecl GetPluginName() { return PLUGIN_NAME_AND_VERSION; }

extern "C" __declspec(dllexport)
PluginObjectType __cdecl GetPluginType() { return(PO_INTERNALS); }

extern "C" __declspec(dllexport)
int __cdecl GetPluginVersion() { return(7); } // InternalsPluginV07 functionality (if you change this return value, you must derive from the appropriate class!)

extern "C" __declspec(dllexport)
PluginObject * __cdecl CreatePluginObject() { return((PluginObject *) new Dx11Plugin); }

extern "C" __declspec(dllexport)
void __cdecl DestroyPluginObject(PluginObject *obj) { delete((Dx11Plugin *)obj); }

using namespace std;

string ws2s(const std::wstring& wstr)
{
	return string(wstr.begin(), wstr.end());
}

void __cdecl add_log(const char *fmt, ...)
{

	if (!fmt || !strlen(fmt))
	{
		return;
	}
	va_list va_alist;
	char logbuf[20000] = { 0 };
	va_start(va_alist, fmt);
	_vsnprintf_s(logbuf + strlen(logbuf), sizeof(logbuf) - strlen(logbuf), _TRUNCATE, fmt, va_alist);
	va_end(va_alist);
	std::ofstream ofile;
	ofile.open("d:\\log.txt", std::ios::app);
	if (ofile)
	{
		ofile << logbuf << std::endl;
		ofile.close();
	}
	return;
}

void Dx11Plugin::Startup(long version)
{
	InitializeHook();
}

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
		pDevice->GetImmediateContext(&pContext);

		FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);
		add_log("Hooked IDXGISwapChain::Present");
		pFW1Factory->Release();
		firstTime = false;
	}
	pFontWrapper->DrawString(pContext, L"So Want To Draw Debug text to screen In DX11 ?", 24.0f, 16.0f, 16.0f, 0xffff1612, FW1_RESTORESTATE);
	
	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

void Dx11Plugin::InitializeHook()
{
	HWND hWnd = GetForegroundWindow();// or FindWindow(rf2 window) 
	IDXGISwapChain* pSwapChain;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;//((GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
	{
		MessageBox(hWnd, L"Failed to create directX device and swapchain!", L"You did what!", MB_ICONERROR);
		return;
	}

	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	PLH::Detour* PresentHook = new PLH::Detour();
	PresentHook->SetupHook((PBYTE)pSwapChainVtable[8] + 0x5, (BYTE*)&hookD3D11Present);
	PresentHook->Hook();
	phookD3D11Present = PresentHook->GetOriginal<D3D11PresentHook>();

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();
}

