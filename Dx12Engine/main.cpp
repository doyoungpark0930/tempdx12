#include "pch.h"
#include "winApp.h"

#pragma comment(lib, "dxguid.lib")

//https://github.com/microsoft/DirectXTK12/wiki/WICTextureLoader
#include <wrl/wrappers/corewrappers.h>
#pragma comment(lib, "runtimeobject.lib")

//directx 12 Agility sdk
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

void CreateConsole();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //메모리 누수 출력(com객체는 안됨)
#endif 

//https://github.com/microsoft/DirectXTK12/wiki/WICTextureLoader
#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize)) throw std::exception();
#else
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) throw std::exception();
#endif

	CreateConsole();
	WinApp::Run(hInstance, nCmdShow, L"DoYeong's Engine", 1280, 720);

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory()); //힙 손상 출력
#endif

	return 0;

}

void CreateConsole()
{
	AllocConsole();  // 콘솔 창 생성
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);  // printf 사용 가능하게 연결
}