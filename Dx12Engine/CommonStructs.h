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

struct maxNode
{
	std::string name;
	Matrix localTransform;
	Matrix offset;
	int mNumChildren = 0;
	maxNode* mChildren[20]; //최대 자식 수 20개
	maxNode* mParent = nullptr;
};

struct TRI_GROUP_PER_MTL
{
	ID3D12Resource* pIndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
	UINT	triCount = 0;

	//material에 맞춰, indexbuffer, view도 동적배열로

	//albedo, normal , metallic 등
	SRV_CONTAINER* srvContainer = nullptr;
	UINT srvNum = 0;
};

struct material 
{
	char* albedoTexFilename = nullptr;
	char* aoTexFilename = nullptr;
	char* normalTexFilename = nullptr;
	char* metallicTexFilename = nullptr;
	char* roughnessTexFilename = nullptr;

	UINT meshNum = 0;
	UINT** index = nullptr; //첫번째 괄호는 mesh순서에 해당

	UINT* face_cnt = nullptr;
	
	~material()
	{
		if (index)  
		{
			for (UINT i = 0; i < meshNum; ++i)
			{
				delete[] index[i]; 
			}
			delete[] index;         
			index = nullptr;
		}
		if (face_cnt)
		{
			delete[] face_cnt;
		}
		if (albedoTexFilename)
		{
			delete[] albedoTexFilename;
		}
		if (aoTexFilename)
		{
			delete[] aoTexFilename;
		}
		if (normalTexFilename)
		{
			delete[] normalTexFilename;
		}
		if (metallicTexFilename)
		{
			delete[] metallicTexFilename;
		}
		if (roughnessTexFilename)
		{
			delete[] roughnessTexFilename;
		}
		
	}
};

struct mesh //일단 이렇게 확실히 mesh로 나누어야함. 이렇게 안하면 각 mesh의 index가 vertex를 참조못함
{
	Vertex* vertices = nullptr;
	UINT verticesNum = 0; 
};

class Animation;
struct MeshDataInfo {
	Vertex* vertices = nullptr;
	UINT verticesNum = 0;

	maxNode* rootNode = nullptr;
	material* Materials = nullptr;
	UINT materialNum = 0;

	mesh* meshes = nullptr;
	UINT meshNum = 0;

	//animation
	Animation* m_animations = nullptr;
	UINT animationCnt = 0;
	Matrix m_defaultTransform;
	Matrix* finalBoneMatrices = nullptr;
	UINT matricesNum = 0;

};