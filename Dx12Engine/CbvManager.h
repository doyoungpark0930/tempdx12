#pragma once

class Renderer;

class CbvManager
{
public:
	CbvManager() = default;
	ID3D12DescriptorHeap* m_descritorHeap = nullptr;
	ID3D12DescriptorHeap* m_animationDescritorHeap = nullptr;
	void OnInit(ID3D12Device* pDevice, Renderer* pRenderer);
	void CreateAnimationBufferPool();
	void Reset();
	CBV_CONTAINER GetGlobalContainer() const { return m_cbvContainer[0]; }
	CBV_CONTAINER* GetAllocContainer();
	CBV_CONTAINER* AllocAnimationMatrices();
	~CbvManager();

	ID3D12Resource* m_constantUploadBufferPool = nullptr;
	UINT8* m_constantBegin = nullptr;    // starting position of upload buffer
	UINT8* m_constantCur = nullptr;      // current position of upload buffer
	UINT8* m_constantEnd = nullptr;      // ending position of upload buffer



	UINT allocatedCbvNum = 1;

private:
	ID3D12Device* m_device = nullptr;
	Renderer* m_renderer = nullptr;
	CBV_CONTAINER* m_cbvContainer = nullptr;

	void UpdateMember();


	UINT descriptorSize = 0;
	const UINT max_descriptorNum = 128;

	//Animation Matrices
	CD3DX12_CPU_DESCRIPTOR_HANDLE animationHandle;
	const UINT max_animationNum = 32;
	CBV_CONTAINER* m_animationContainer = nullptr;
	UINT m_animationContainerCnt = 0;
	ID3D12Resource* m_animationConstantUploadBufferPool = nullptr; 
	UINT8* m_animationConstantBegin = nullptr;  
	UINT8* m_animationConstantCur = nullptr;     
	UINT8* m_animationConstantEnd = nullptr;    
};