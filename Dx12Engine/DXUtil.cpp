#include "pch.h"
#include "DXUtil.h"


wchar_t DXUtil::m_assetsResourcesPath[512] = L"";

// Helper function for resolving the full path of assets.
void DXUtil::GetResourcesPathName(const wchar_t* assetName, wchar_t* outBuffer, size_t bufSize)
{
	wcscpy_s(outBuffer, bufSize, m_assetsResourcesPath);
	wcscat_s(outBuffer, bufSize, assetName);

}

void GetHardwareAdapter(
	IDXGIFactory1* pFactory,
	IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;

	IDXGIAdapter1* adapter = nullptr;


	IDXGIFactory6* factory6;
	//window 10이상만 factory6지원
	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (
			UINT adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(&adapter)));
				adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			wprintf(L"Adapter %d: %s\n", adapterIndex, desc.Description);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}
			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	//window 10 미만일 때
	if (adapter == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*ppAdapter = adapter;

	if (factory6)
	{
		factory6->Release();

		factory6 = nullptr;
	}
}


//https://learn.microsoft.com/en-us/windows/win32/direct3d12/uploading-resources
HRESULT SuballocateFromBuffer(UINT8** m_pDataCur, UINT8* m_pDataEnd, SIZE_T uSize, UINT uAlign)
{
	//*m_pDataCur의 주소값을 uAlign의 배수로 맞춤
	*m_pDataCur = reinterpret_cast<UINT8*>(
		Align(reinterpret_cast<SIZE_T>(*m_pDataCur), uAlign)
		);

	return (*m_pDataCur + uSize > m_pDataEnd) ? E_INVALIDARG : S_OK;
}

//
// Place and copy data to the upload buffer.
//

HRESULT SetDataToUploadBuffer(
	UINT8** m_pDataCur,
	UINT8* m_pDataBegin,
	UINT8* m_pDataEnd,
	const void* pData,
	UINT bytesPerData,
	UINT dataCount,
	UINT alignment,
	UINT& byteOffset
)
{
	SIZE_T byteSize = bytesPerData * dataCount;
	HRESULT hr = SuballocateFromBuffer(m_pDataCur, m_pDataEnd, byteSize, alignment);
	if (SUCCEEDED(hr))
	{
		byteOffset = UINT(*m_pDataCur - m_pDataBegin);
		if(pData) memcpy(*m_pDataCur, pData, byteSize);
		*m_pDataCur += byteSize;
	}

	return hr;
}

//
// Align uLocation to the next multiple of uAlign.
//

SIZE_T Align(SIZE_T uLocation, SIZE_T uAlign)
{
	if ((0 == uAlign) || (uAlign & (uAlign - 1))) // uAlign이 0이거나 2의 거듭제곱(1,2,4,8...)이 아니면 예외 발생
	{
		throw std::exception();
	}

	// uLocation을 uAlign 배수로 올림 (정렬)
   // 예: uAlign = 256, uLocation = 8 → 256 반환
   // 비트 마스크를 이용하여 가장 가까운 uAlign 배수로 맞춘다.
	return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
}