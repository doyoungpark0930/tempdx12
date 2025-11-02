#pragma once

class Renderer;
class DescriptorPool;
class SrvManager;
class CbvManger;
class Animation;

class Model
{
public:
	static Renderer* m_renderer;
	static DescriptorPool* m_descriptorPool;
	static SrvManager* m_srvManager;
	static CbvManager* m_cbvManager;
	void CreateModel(MeshDataInfo meshesInfo, bool useNormalMap);
	void CreateTextureFromName(char* textureFilename, SRV_CONTAINER* srvContainer, UINT* srvCnt);
	D3D12_VERTEX_BUFFER_VIEW CreateVertexBuffer(Vertex* vertices, UINT vertexCount);
	D3D12_INDEX_BUFFER_VIEW CreateIndexBuffer(UINT* indices, UINT indiceCount);
	void Draw(const Matrix* pMatrix);
	Model();
	~Model();

private:
	ID3D12Device* m_device = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	UINT64* m_fenceValue = nullptr;

	UINT descriptorSize;


	UINT meshNum = 0;
	Mesh* m_Meshes = nullptr;
	UINT texNum = 5;

	//animation
	bool existAnimation = false;
	CBV_CONTAINER* boneMatricesContainer = nullptr;
	Matrix* m_FinalBoneMatrices = nullptr;
	UINT m_matricesNum = 0;
	Animation* m_animations = nullptr;

	//직접 입력해야하는 텍스춰라면
	bool m_hasManualTextureInput = false;
	SRV_CONTAINER* m_manualTextureContainer = nullptr;
	UINT m_menualSrvCnt = 0;

	void UpdateMembers();
	void WaitForPreviousFrame();

	bool m_useNormalMap = false;

	//boneNode
	maxNode* rootNode = nullptr;


};