#pragma once

class Model;
class DescriptorPool;
class SrvManager;
class CbvManager;
class Animator;

class Renderer
{
public:
	void OnInit();
	void Update(float dt);
	void Render();
	void ObjectRender();


	UINT GetWidth() const { return clientWidth; }
	UINT GetHeight() const { return clientHeight; }

	ID3D12Device* GetDevice() const { return m_device; }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList; }
	ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator; }
	ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue; }
	ID3D12PipelineState* GetPipeline() const { return m_pipelineState; }
	DescriptorPool* GetDescriptorPool() const { return m_descriptorPool; }
	ID3D12Fence* GetFence() const { return m_fence; }
	HANDLE GetFenceEvent() const { return m_fenceEvent; }
	UINT64* GetFenceValue() { return &m_fenceValue; }

	ID3D12Resource* m_vsBufferPool = nullptr;
	ID3D12Resource* m_vsUploadBufferPool = nullptr;
	UINT8* m_vsBegin = nullptr;    // starting position of upload buffer
	UINT8* m_vsCur = nullptr;      // current position of upload buffer
	UINT8* m_vsEnd = nullptr;      // ending position of upload buffer

	ID3D12Resource* m_indexBufferPool = nullptr;
	ID3D12Resource* m_indexUploadBufferPool = nullptr;
	UINT8* m_indexBegin = nullptr;    // starting position of upload buffer
	UINT8* m_indexCur = nullptr;      // current position of upload buffer
	UINT8* m_indexEnd = nullptr;



	Renderer(UINT width, UINT height);
	~Renderer();

private:
	static const UINT FrameCount = 2;

	void CreateRootSignature();
	void CreatePipelineState();
	void CreateDepthStencil();
	void CreateObjects();
	void Create_Vertex_Index();
	void CreateModels();
	void inputTexFilename(char* basePath, const char* fileName, char** TexFileToInput);
	void OnInitGlobalConstant();

	UINT clientWidth;
	UINT clientHeight;


	// Pipeline objects.
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	ID3D12Resource* m_renderTargets[FrameCount] = {};
	ID3D12Resource* m_depthStencil = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	DescriptorPool* m_descriptorPool = nullptr;
	SrvManager* m_srvManager = nullptr;
	CbvManager* m_cbvManager = nullptr;
	UINT m_rtvDescriptorSize;


	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence;
	UINT64 m_fenceValue;

	Model* m_Models = nullptr;
	ObjectState* m_ObjectState = nullptr;
	UINT maxObjectsNum = 256; //최대 오브젝트의 수
	UINT ObjectCnt = -1;  //오브젝트가 몇 번째로 생성되었는지

	Matrix View;
	Matrix Proj;

	FLOAT fovY = XM_PIDIV4;
	FLOAT aspect;
	FLOAT nearZ = 0.1f;
	FLOAT farZ = 100.0f;

	Animator* m_animator = nullptr;

	Vector3 eyePos = Vector3(0.0f, 0.0f, -5.0f);
	Vector3 lookAt = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	//Vector3 lightPos = Vector3(1.0f, 0.5f, -0.5f);
	Vector3 lightPos = Vector3(0.0f, 0.5f, -2.0f);


	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

};