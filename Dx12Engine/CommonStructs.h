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

struct material // material* Materials = new [50];만들고, mtl읽을때 다 채워넣는다
{//face에서 mtlid만나면, Materials[id]->index[face_cnt*3+0], [face_cnt*3+1],[face_cnt*3+2]에 index값 넣어주고, face_cnt++;해주기
	//emission이라든지 여기에 없는 char*가 들어오면 그거는 무시하고 안 넣으면 됨
	char* albedoTexFilename = nullptr;
	char* aoTexFilename = nullptr;
	char* normalTexFilename = nullptr;
	char* metallicTexFilename = nullptr;
	char* roughnessTexFilename = nullptr;

	UINT meshNum = 0;
	UINT** index = nullptr; //2차원배열. 일단 이렇게 되나 보고, 이거 vector pushback 내가 직접만들어야겟다
	//mesh[0] -> index 1, 2 ,3 ,4, 5, 6
	//mesh[1] -> index 1, 2 ,3 ,4, 5, 6
	//mesh[2] -> index 1, 2 ,3 ,4, 5, 6
	//face cnt는 6개
	//IASetVertex[0]할당하고 index쫙
	//IASetVertex[1]할당하고 index쫙 => 이것들을 tri로는 어떻게 잘 넣을지 생각

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
	UINT indexNum = 0;
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