#pragma once

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	Vector3 Pos;
	Vector3 Normal;
	Vector2 Tex;
	Vector3 tangent;

	FLOAT m_Weights[MAX_BONE_INFLUENCE];
	uint8_t m_BoneIDs[MAX_BONE_INFLUENCE];

};

struct BoneInfo
{
	int id;
	Matrix offset;
};

struct NormalVertex
{
	Vector3 Pos;
	Vector3 Normal;
	Vector3 StartPos;  //vertex시작 지점
};

struct GLOBAL_CONSTANT
{
	Matrix ViewProj;
	Vector4 eyePos;
	Vector4 lightPos;
};

struct MODEL_CONSTANT
{
	Matrix Model;
	Matrix NormalModel;
	UINT useNormalMap;
};

struct SkinnedConstants
{
	Matrix boneTransforms[64];
};

struct ObjectState
{
	Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
	Vector3 rotation = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
};

struct SRV_CONTAINER
{
	ID3D12Resource* pSrvResource;
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;
};

struct CBV_CONTAINER
{
	UINT8* pSystemMemAddr;
	D3D12_CPU_DESCRIPTOR_HANDLE	CBVHandle;
};

struct Mesh
{
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	SRV_CONTAINER* srvContainer = nullptr;

	UINT indexCount = 0;
	UINT srvNum = 0;
};

struct MeshData {
	Vertex* vertices = nullptr;
	UINT* indices = nullptr;

	UINT verticesNum = 0;
	UINT indicesNum = 0;
};

struct maxNode
{
	std::string name;
	Matrix localTransform;
	Matrix offset;
	int mNumChildren = 0;
	maxNode* mChildren[5]; //최대 자식 수 5개
	maxNode* mParent = nullptr;
};

class Animation;
struct MeshDataInfo {
	MeshData* meshes = nullptr;
	UINT meshCount = 1;
	Animation* m_animations = nullptr;
	UINT animationCnt = 0;
	Matrix m_defaultTransform;
	Matrix* finalBoneMatrices = nullptr;
	UINT matricesNum = 0;

	maxNode* rootNode = nullptr;

	//직접 입력하는 대신, 텍스춰파일하나를 모든 메쉬가 참조하는 구조
	char* albedoTexFilename = nullptr;
	char* aoTexFilename = nullptr;
	char* normalTexFilename = nullptr;
	char* metallicTexFilename = nullptr;
	char* roughnessTexFilename = nullptr;

	//직접 입력해야하는 텍스춰라면
	bool hasManualTextureInput = false;
};