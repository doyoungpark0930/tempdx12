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
	char* ReadTextureFilename(aiMaterial* material, UINT type);
	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }

public:
	char* basePath;
	Animation* m_animations = nullptr;
	Matrix defaultTransform;

	maxNode* rootNode = nullptr;
	material* m_materials = nullptr;
	mesh* m_meshes = nullptr;

	UINT m_materialNum = 0;
	UINT m_meshesNum = 0;
	int m_noSubMtlNum = 0;

	int meshCnt = 0; //Geometry들어올때 몇번째 mesh인지

private:

	std::unordered_map<string, BoneInfo> m_BoneInfoMap;
	std::unordered_map<string, UINT> m_boneInfoMap;
	int m_BoneCounter = 0;
	void InsertMtlValue(int allocatedMeterialNum, std::string& mapName, std::string& mapPath);
	void ReadMapInfo(FILE* pStream, int allocatedMaterialNum);
	inline void ParseVertexLine(const char* startPlace, Vertex* vertex, int meshCnt, int mtlID, int index);
	inline void ParseBoneWeights(char* line, Vertex* vertex);
	void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_Weights[i] = 0.0f;
			vertex.m_BoneIDs[i] = 255;
		}
	}

};


