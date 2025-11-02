#pragma once

class Renderer;

class SrvManager
{
public :
	SrvManager() = default;
	ID3D12DescriptorHeap* m_descritorHeap = nullptr;
	void OnInit(ID3D12Device* pDevice, Renderer* pRenderer);
	SRV_CONTAINER CreateTiledTexture();
	SRV_CONTAINER CreateTexture(const wchar_t* szFileName);
	UINT8* GenerateTextureData();
	UINT GetAllocatedNum() const { return allocatedNum; }
	SRV_CONTAINER* GetSRVContainer() const { return m_srvContainer; }
	~SrvManager();

private:
	ID3D12Device* m_device = nullptr;
	Renderer* m_renderer = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	UINT64* m_fenceValue = nullptr;

	UINT descriptorSize = 0;
	ID3D12Resource* m_textureUploadHeap = nullptr;
	bool uploadHeapCreated = false;
	const UINT max_descriptorNum = 256;
	ID3D12Resource** m_textures = nullptr;

	UINT allocatedNum = 0;
	SRV_CONTAINER* m_srvContainer = nullptr;

	const UINT TextureWidth = 256;
	const UINT TextureHeight = 256;
	const UINT TexturePixelSize = 4;

	void UpdateMember();
	void WaitForPreviousFrame();

};