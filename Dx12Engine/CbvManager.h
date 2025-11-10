#pragma once

class Renderer;

class CbvManager
{
public:
	CbvManager() = default;
	ID3D12DescriptorHeap* m_descritorHeap = nullptr;
	ID3D12DescriptorHeap* m_materialDescritorHeap = nullptr;
	ID3D12DescriptorHeap* m_animationDescritorHeap = nullptr;
	void OnInit(ID3D12Device* pDevice, Renderer* pRenderer);
	void Reset();
	CBV_CONTAINER GetGlobalContainer() const { return m_cbvContainer[0]; }
	CBV_CONTAINER* GetAllocatedContainer();
	CBV_CONTAINER AllocMaterialCBV();
	CBV_CONTAINER* AllocAnimationMatrices();
	UINT8* GetStartCBV() const { return m_constantBegin; }
	~CbvManager();



private:
	ID3D12Device* m_device = nullptr;
	Renderer* m_renderer = nullptr;
	CBV_CONTAINER* m_cbvContainer = nullptr;

	void UpdateMember();
	void CreateAnimationBufferPool();
	void CreateMaterialBufferPool();

	UINT descriptorSize = 0;
	const UINT max_descriptorNum = 128;

	//global, model Constant
	ID3D12Resource* m_constantUploadBufferPool = nullptr;
	UINT8* m_constantBegin = nullptr;    // starting position of upload buffer
	UINT8* m_constantCur = nullptr;      // current position of upload buffer
	UINT8* m_constantEnd = nullptr;      // ending position of upload buffer
	UINT allocatedCbvNum = 1;

	//Material Constant
	ID3D12Resource* m_materialConstantUploadBufferPool = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE materialHandle;
	const UINT max_materialNum = 128;
	CBV_CONTAINER* m_materialContainer = nullptr;
	UINT m_materialContainerCnt = 0;
	UINT8* m_materialConstantBegin = nullptr;
	UINT8* m_materialConstantCur = nullptr;
	UINT8* m_materialConstantEnd = nullptr;


	//Animation Matrices
	ID3D12Resource* m_animationConstantUploadBufferPool = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE animationHandle;
	const UINT max_animationNum = 32;
	CBV_CONTAINER* m_animationContainer = nullptr;
	UINT m_animationContainerCnt = 0;
	UINT8* m_animationConstantBegin = nullptr;  
	UINT8* m_animationConstantCur = nullptr;     
	UINT8* m_animationConstantEnd = nullptr;    
};