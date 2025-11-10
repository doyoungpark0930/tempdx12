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
	void CreateModel(MeshDataInfo meshesInfo);
	void CreateTextureFromName(char* textureFilename, SRV_CONTAINER& srvContainer);
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

	//TriGroup
	D3D12_VERTEX_BUFFER_VIEW* m_vertexBufferView = nullptr;
	TRI_GROUP_PER_MTL* m_TriGroupList = nullptr;
	UINT m_materialNum = 0;
	UINT m_meshNum = 0;

	//MaterialConstant
	CBV_CONTAINER* materialContainer = nullptr;

	//animation
	bool existAnimation = false;
	CBV_CONTAINER* boneMatricesContainer = nullptr;
	Matrix* m_FinalBoneMatrices = nullptr;
	UINT m_matricesNum = 0;
	Animation* m_animations = nullptr;


	void UpdateMembers();
	void WaitForPreviousFrame();

	bool m_useNormalMap = false;

	//boneNode
	maxNode* rootNode = nullptr;


};