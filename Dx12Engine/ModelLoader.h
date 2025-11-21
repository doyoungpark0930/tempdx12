#pragma once
#include <unordered_map>
#include <string>

using namespace std;

class Animation;


class ModelLoader {
public:
	~ModelLoader();
	void Load(char* basePath, const char* filename);
	void UpdateResourcePath(material* m_materials, UINT m_materialNum);
	char* UpdatePath(char* filePath);
	auto GetBoneInfoMap() { return  m_BoneInfoMap; }
	int GetBoneCount() { return m_BoneCounter; }

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
	std::unordered_map<std::string, UINT> m_BoneInfoMap;
	int m_BoneCounter = 0;
	void InsertMtlValue(int allocatedMeterialNum, std::string& mapName, std::string& mapPath);
	void ReadMapInfo(FILE* pStream, int allocatedMaterialNum);
	inline void ParseVertexLine(const char* startPlace, Vertex* vertex, int meshCnt, int mtlID, int index);
	inline void ParseBoneWeights(char* line, Vertex* vertex);
	void PrintAllMaterialTextures(material* m_materials, UINT m_materialNum);
	void printNode(maxNode* node, int depth = 0);
	maxNode* findNode(maxNode* node, const std::string& name);

	void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_Weights[i] = 0.0f;
			vertex.m_BoneIDs[i] = 255;
		}
	}

};


