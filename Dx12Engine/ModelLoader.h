#pragma once


#include <vector> //나중에 빼야함
#include <unordered_map>
#include <string>
using namespace std;

struct aiMaterial;
struct aiNode;
struct aiMesh;
struct aiScene;
class Animation;



class ModelLoader {
public:
	~ModelLoader();
	void testLoad(char* basePath, const char* filename);
	void deleteNode(maxNode* node);
	char* ReadTextureFilename(aiMaterial* material, UINT type);
	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }

public:
	char* basePath;
	MeshData* meshData;
	Animation* m_animations = nullptr;
	Matrix defaultTransform;
	maxNode* rootNode = nullptr;

	UINT m_vertexNum = 0;
	UINT m_indexNum = 0;

private:

	std::unordered_map<string, BoneInfo> m_BoneInfoMap;
	std::unordered_map<string, UINT> m_boneInfoMap;
	int m_BoneCounter = 0;

	void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_Weights[i] = 0.0f;
			vertex.m_BoneIDs[i] = 255;
		}
	}

};


