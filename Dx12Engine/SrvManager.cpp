#include "pch.h"

#include "WICTextureLoader12.h"
#include "DDSTextureLoader12.h"
#include "Renderer.h"
#include "SrvManager.h"


void SrvManager::UpdateMember()
{
	m_commandList = m_renderer->GetCommandList();
	m_commandAllocator = m_renderer->GetCommandAllocator();
	m_commandQueue = m_renderer->GetCommandQueue();
	m_pipelineState = m_renderer->GetPipeline();
	m_fence = m_renderer->GetFence();
	m_fenceEvent = m_renderer->GetFenceEvent();
	m_fenceValue = m_renderer->GetFenceValue();
}
void SrvManager::OnInit(ID3D12Device* pDevice, Renderer* pRenderer)
{
	m_device = pDevice;
	m_renderer = pRenderer;

	UpdateMember();

	descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = max_descriptorNum;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	if (FAILED(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_descritorHeap)))) __debugbreak();

	m_srvContainer = new SRV_CONTAINER[max_descriptorNum];
	m_textures = new ID3D12Resource * [max_descriptorNum];

}
SRV_CONTAINER SrvManager::CreateTiledTexture()
{
	UpdateMember();

	// Describe and create a Texture2D.
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width = TextureWidth;
	textureDesc.Height = TextureHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
	if (FAILED(m_device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_textures[allocatedNum]))))__debugbreak();

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textures[allocatedNum], 0, 1);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	if (FAILED(m_device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_textureUploadHeap)))) __debugbreak();


	// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the Texture2D.
	UINT8* texture = GenerateTextureData();

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = texture;
	textureData.RowPitch = TextureWidth * TexturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * TextureHeight;


	if (FAILED(m_commandAllocator->Reset())) __debugbreak();
	if (FAILED(m_commandList->Reset(m_commandAllocator, m_pipelineState))) __debugbreak();

	UpdateSubresources(m_commandList, m_textures[allocatedNum], m_textureUploadHeap, 0, 0, 1, &textureData);
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_textures[allocatedNum], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	if (FAILED(m_commandList->Close())) __debugbreak();

	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForPreviousFrame();

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descritorHeap->GetCPUDescriptorHandleForHeapStart());
	srvHandle.Offset(allocatedNum, descriptorSize);
	m_device->CreateShaderResourceView(m_textures[allocatedNum], &srvDesc, srvHandle);

	m_srvContainer[allocatedNum].pSrvResource = m_textures[allocatedNum];
	m_srvContainer[allocatedNum].srvHandle = srvHandle;

	if (texture)
	{
		delete[] texture;
		texture = nullptr;
	}

	if (m_textureUploadHeap)
	{
		m_textureUploadHeap->Release();
		m_textureUploadHeap = nullptr;
	}
	
	return m_srvContainer[allocatedNum++];
}
SRV_CONTAINER SrvManager::CreateTexture(const wchar_t* szFileName)
{
	UpdateMember();

	bool useDDS;
	std::unique_ptr<uint8_t[]> Datas;
	std::vector<D3D12_SUBRESOURCE_DATA> ddsSubresources; //dds에서 사용하는 Subresoruces
	D3D12_SUBRESOURCE_DATA wicSubresource;//wic에서 사용하는 Subresource

	const wchar_t* lastSlash = wcsrchr(szFileName, L'.');

	if (wcscmp(lastSlash, L".dds") == 0)
	{
		useDDS = true;
		if (FAILED(LoadDDSTextureFromFile(m_device, szFileName, &m_textures[allocatedNum], Datas, ddsSubresources)))
		{
			printf("Failed to load DDS texture file by SrvManager::CreateTexture\n");
			throw std::exception();
		}
	}
	else
	{
		useDDS = false;
		if (FAILED(LoadWICTextureFromFile(m_device, szFileName, &m_textures[allocatedNum], Datas, wicSubresource)))
		{
			printf("Failed to load WIC texture file by SrvManager::CreateTexture\n");
			throw std::exception();
		}
	}
	
	UINT64 uploadBufferSize;
	if(useDDS) uploadBufferSize = GetRequiredIntermediateSize(m_textures[allocatedNum], 0, UINT(ddsSubresources.size()));
	else uploadBufferSize = GetRequiredIntermediateSize(m_textures[allocatedNum], 0, 1);

	// Create the GPU upload buffer.
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	if (FAILED(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_textureUploadHeap)))) __debugbreak();

	if (FAILED(m_commandAllocator->Reset())) __debugbreak();
	if (FAILED(m_commandList->Reset(m_commandAllocator, m_pipelineState))) __debugbreak();

	if (useDDS) UpdateSubresources(m_commandList, m_textures[allocatedNum], m_textureUploadHeap, 0, 0, UINT(ddsSubresources.size()), ddsSubresources.data());
	else UpdateSubresources(m_commandList, m_textures[allocatedNum], m_textureUploadHeap, 0, 0, 1, &wicSubresource);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_textures[allocatedNum], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	if (FAILED(m_commandList->Close())) __debugbreak();
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	WaitForPreviousFrame();

	
	D3D12_RESOURCE_DESC ddsDesc = m_textures[allocatedNum]->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = ddsDesc.Format; 
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = ddsDesc.MipLevels; 
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descritorHeap->GetCPUDescriptorHandleForHeapStart());
	srvHandle.Offset(allocatedNum, descriptorSize);
	m_device->CreateShaderResourceView(m_textures[allocatedNum], &srvDesc, srvHandle);

	m_srvContainer[allocatedNum].pSrvResource = m_textures[allocatedNum];
	m_srvContainer[allocatedNum].srvHandle = srvHandle;

	if (m_textureUploadHeap)
	{
		m_textureUploadHeap->Release();
		m_textureUploadHeap = nullptr;
	}

	return m_srvContainer[allocatedNum++];
}

void SrvManager::WaitForPreviousFrame()
{
	// Signal and increment the fence value.
	const UINT64 fence = *m_fenceValue;
	if (FAILED(m_commandQueue->Signal(m_fence, fence))) __debugbreak();
	(*m_fenceValue)++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		if (FAILED(m_fence->SetEventOnCompletion(fence, m_fenceEvent))) __debugbreak();
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

}

UINT8* SrvManager::GenerateTextureData()
{
	const UINT rowPitch = TextureWidth * TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;

	UINT8* pData = new UINT8[textureSize];

	for (UINT n = 0; n < textureSize; n += TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0xff;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	return pData;
}

SrvManager::~SrvManager()
{
	if (m_textureUploadHeap)
	{
		m_textureUploadHeap->Release();
		m_textureUploadHeap = nullptr;
	}
	if (m_textures)
	{
		for (int i = 0; i < allocatedNum; i++)
		{
			if (m_textures[i])
			{
				m_textures[i]->Release();
				m_textures[i] = nullptr;
			}
		}
		delete[] m_textures;
		m_textures = nullptr;
	}
	if (m_srvContainer)
	{
		delete[] m_srvContainer;
		m_srvContainer = nullptr;

	}
	if (m_descritorHeap)
	{
		m_descritorHeap->Release();
		m_descritorHeap = nullptr;
	}
}