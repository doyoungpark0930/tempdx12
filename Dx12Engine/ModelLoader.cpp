#include "pch.h"

// Assimp에서 new매크로와 메모리누수의 new와 충돌 제거하기 위함(pch.h에 정의됨)
#ifdef new
#undef new  
#endif

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


#pragma comment(lib, "assimp-vc143-mtd.lib") 


//메모리 릭 확인 재정의
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "ModelLoader.h"
#include "Animation.h"
#include "DirectXMesh.h"
#include "DXHelper.h"
#include "CommonStructs.h"

#pragma comment(lib, "DirectXMesh.lib") 
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cctype>

using namespace std;
using namespace DirectX::SimpleMath;


bool fgets_trim(char* buffer, size_t size, FILE* stream)
{
	if (!fgets(buffer, size, stream))
		return false;

	size_t len = strlen(buffer);
	if (len > 0 && buffer[len - 1] == '\n')
		buffer[len - 1] = '\0';

	return true;
}

void FindStr(char* startPlace, std::string& dest)
{
	char* start = strchr(startPlace, '"');
	if (start)
	{
		start++; // 큰따옴표 다음 문자

		char* end = strchr(start, '"');
		if (end)
		{
			dest.assign(start, end - start); // 문자열 구간 복사
		}
	}
}

inline bool checkTitle(char* line, const char* str, int& currentIndent)
{
	char* start = strstr(line, str);

	if (start) {
		currentIndent = start - line;
		return true;
	}
	return false;

}

maxNode* findNode(maxNode* node, const std::string& name)
{
	if (node->name == name) return node;

	maxNode* targetNode = nullptr;
	for (int i = 0; i < node->mNumChildren; i++)
	{
		targetNode = findNode(node->mChildren[i], name);
		if (targetNode) return targetNode;
	}

	return nullptr;
}

void  ModelLoader::deleteNode(maxNode* node)
{
	if (node == nullptr)
		return;

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		deleteNode(node->mChildren[i]);
		node->mChildren[i] = nullptr;
	}

	delete node;
}


void printNode(maxNode* node, int depth = 0)
{
	if (!node)
		return;

	// 부모-자식 구조 들여쓰기
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";

	std::cout << "└─ " << node->name << " (" << node->mNumChildren << " children)" << std::endl;

	// transformation 출력 (4x4)
	for (int r = 0; r < 4; ++r)
	{
		for (int i = 0; i < depth + 1; ++i) // 들여쓰기
			std::cout << "  ";

		for (int c = 0; c < 4; ++c)
		{
			std::cout << node->localTransform.m[r][c] << " ";
		}
		std::cout << std::endl;
	}

	// 자식 노드 재귀 출력
	for (int i = 0; i < node->mNumChildren; ++i)
	{
		printNode(node->mChildren[i], depth + 1);
	}
}


void ParseVertexLine(const char* startPlace, Vertex& vertex)
{
	float x[11];
	//position, uv, normal, tangent순
	if (sscanf_s(startPlace, "%f %f %f %f %f %f %f %f %f %f %f",
		&x[0], &x[1], &x[2], &x[3], &x[4],
		&x[5], &x[6], &x[7], &x[8], &x[9], &x[10]) == 11)
	{
		vertex.Pos = { x[0], x[1], x[2] };
		vertex.Tex = { x[3], x[4] };
		vertex.Normal = { x[5], x[6], x[7] };
		vertex.tangent = { x[8], x[9], x[10] };
	}
}

char* findNthNumberStart(char* line, int n)
{
	char* ptr = line;
	int count = 0;

	while (*ptr != '\0') // 문자열 끝날 때까지
	{
		// 숫자나 '-'(음수 표시)가 나오면 숫자의 시작 가능성
		if (isdigit(*ptr) || *ptr == '-' || *ptr == '.')
		{
			// 앞이 공백이거나 문자열의 시작이라면 진짜 숫자 시작
			if (ptr == line || isspace(*(ptr - 1)))
			{
				count++;
				if (count == n)
					return ptr; // n번째 숫자의 시작 주소 반환
			}
		}

		ptr++;
	}

	return nullptr; // 못 찾은 경우
}

void ParseBoneWeights(char* line, Vertex& vertex)
{
	int weightNum = 0;
	char* startPlace = nullptr;
	if (sscanf_s(line, "%d", &weightNum) == 1)
	{
		float weight = 0.0f;
		int boneID = 255;
		if (weightNum != 0) startPlace = findNthNumberStart(line, 2); // 첫 float 앞
		for (int i = 0; i < weightNum; i++)
		{
			if (sscanf_s(startPlace, "%f %d", &weight, &boneID) == 2)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = static_cast<uint8_t>(boneID);
			}

			startPlace = findNthNumberStart(startPlace, 3); // 다음 float,int 쌍
		}
	}
}

void ModelLoader::testLoad(char* basePath, const char* filename)
{
	this->basePath = basePath;
	char fullPath[512];
	strcpy_s(fullPath, sizeof(fullPath), basePath);
	strcat_s(fullPath, sizeof(fullPath), filename);
	meshData = new MeshData;
	FILE* pStream = nullptr;
	fopen_s(&pStream, fullPath, "r");

	if (!pStream) {
		cout << "file can't be opened" << endl;
		return;
	}

	char line[512];

	//Indent
	int MATERIAL_Indent = 0;
	int MAP_GENERIC_indent = 0;
	int HELPER_indent = 0;
	int GEOM_indent = 0;
	int MESH_indent = 0;
	int FACE_indent = 0;
	int BONE_indent = 0;

	char* startPlace = nullptr; //문자열 시작 주소 파악

	//HelperObject Node만들 때 사용하는 포인터
	maxNode* curNode = nullptr;
	maxNode* beforeNode = nullptr;

	int test = 0;
	while (fgets_trim(line, sizeof(line), pStream)) {

		//HelperObject
		if (checkTitle(line, "*HELPEROBJECT", HELPER_indent))
		{
			while (fgets_trim(line, sizeof(line), pStream))
			{

				startPlace = strstr(line, "*NODE_NAME");
				if (startPlace != nullptr) {
					std::string nodeName;
					FindStr(startPlace, nodeName);
					maxNode* node = new maxNode;
					beforeNode = curNode;
					if (!beforeNode) rootNode = node;
					curNode = node;
					curNode->name = nodeName;
					continue;
				}

				startPlace = strstr(line, "*NODE_PARENT");
				if (startPlace != nullptr) {
					std::string parentName;
					FindStr(startPlace, parentName);

					if (!beforeNode) __debugbreak();
					if (beforeNode->name == parentName)
					{
						curNode->mParent = beforeNode;

						int childNum = beforeNode->mNumChildren;
						beforeNode->mChildren[childNum] = curNode;
						beforeNode->mNumChildren++;
					}
					else
					{
						maxNode* parentNode = findNode(rootNode, parentName);
						if (!parentNode) __debugbreak();
						curNode->mParent = parentNode;

						int childNum = parentNode->mNumChildren;
						parentNode->mChildren[childNum] = curNode;
						parentNode->mNumChildren++;
					}
					continue;
				}

				int TRANSFORM_indent = 0;
				int localRow = 0;
				int offsetRow = 0;
				if (checkTitle(line, "*NODE_TRANSFORMATION_MATRIX", TRANSFORM_indent))
				{
					while (fgets_trim(line, sizeof(line), pStream))
					{

						startPlace = strstr(line, "*LOCAL_TM");
						if (startPlace != nullptr)
						{
							startPlace += strlen("*LOCAL_TM") + 1;
							float x[3];
							if (curNode && (sscanf_s(startPlace, "%f %f %f", &x[0], &x[1], &x[2]) == 3))
							{
								curNode->localTransform.m[localRow][0] = x[0];
								curNode->localTransform.m[localRow][1] = x[1];
								curNode->localTransform.m[localRow][2] = x[2];
								curNode->localTransform.m[localRow][3] = 0.0f;

								curNode->localTransform.m[3][3] = 1.0f;
							}
							else __debugbreak();
							localRow++;
							continue;
						}

						startPlace = strstr(line, "*Offset_TM");
						if (startPlace != nullptr)
						{
							startPlace += strlen("*Offset_TM") + 1;
							float x[3];
							if (curNode && (sscanf_s(startPlace, "%f %f %f", &x[0], &x[1], &x[2]) == 3))
							{
								curNode->offset.m[offsetRow][0] = x[0];
								curNode->offset.m[offsetRow][1] = x[1];
								curNode->offset.m[offsetRow][2] = x[2];
								curNode->offset.m[offsetRow][3] = 0.0f;

								curNode->offset.m[3][3] = 1.0f;
							}
							else __debugbreak();
							offsetRow++;
							continue;
						}

						if (line[TRANSFORM_indent] == '}')
						{
							break;
						}
					}
				}

				if (line[HELPER_indent] == '}')
				{
					break;
				}
			}
			continue;
		}


		//GeometryObject
		if (checkTitle(line, "*GEOMOBJECT", GEOM_indent))
		{
			while (fgets_trim(line, sizeof(line), pStream))
			{
				if (checkTitle(line, "*MESH", MESH_indent))
				{
					while (fgets_trim(line, sizeof(line), pStream))
					{

						startPlace = strstr(line, "*MESH_NUMFACES");
						if (startPlace != nullptr)
						{
							startPlace += strlen("*MESH_NUMFACES");
							int x;
							if (sscanf_s(startPlace, "%d", &x) == 1)
							{
								meshData->verticesNum = x * 3;
								meshData->indicesNum = x * 3;
								meshData->vertices = new Vertex[x * 3];
								meshData->indices = new UINT[x * 3];

								for (int i = 0; i < x * 3; i++)
								{
									meshData->indices[i] = i;
									SetVertexBoneDataToDefault(meshData->vertices[i]);
								}

							}
							continue;
						}

						if (checkTitle(line, "*MESH_FACE_LIST", FACE_indent))
						{
							int index = 0;
							while (fgets_trim(line, sizeof(line), pStream))
							{
								if (line[FACE_indent] == '}')
								{
									break;
								}
								fgets_trim(line, sizeof(line), pStream); //mtl ID

								fgets_trim(line, sizeof(line), pStream); //vertex
								ParseVertexLine(line, meshData->vertices[index]);
								fgets_trim(line, sizeof(line), pStream); //weight
								ParseBoneWeights(line, meshData->vertices[index]);
								index++;

								fgets_trim(line, sizeof(line), pStream);//vertex
								ParseVertexLine(line, meshData->vertices[index]);
								fgets_trim(line, sizeof(line), pStream);//weight
								ParseBoneWeights(line, meshData->vertices[index]);
								index++;

								fgets_trim(line, sizeof(line), pStream);//vertex
								ParseVertexLine(line, meshData->vertices[index]);
								fgets_trim(line, sizeof(line), pStream);//weight
								ParseBoneWeights(line, meshData->vertices[index]);
								index++;
							}
						}

						if (line[MESH_indent] == '}')
						{
							break;
						}
					}
				}


				if (line[GEOM_indent] == '}')
				{
					break;
				}
			}
		}

		if (checkTitle(line, "BONE_LIST", BONE_indent))
		{
			std::string boneName;
			int boneId = 0;
			while (fgets_trim(line, sizeof(line), pStream))
			{
				FindStr(line, boneName);
				if (sscanf_s(line + boneName.size()+3, "%d", &boneId) == 1)
				{
					m_boneInfoMap[boneName] = boneId;
				}

				if (line[BONE_indent] == '}')
				{
					break;
				}
			}
		}

	}



	//printNode(rootNode, 0);

	fclose(pStream);
}








char* ModelLoader::ReadTextureFilename(aiMaterial* material, UINT type)
{
	if (material->GetTextureCount(static_cast<aiTextureType>(type)) > 0) {
		aiString filepath;
		material->GetTexture(static_cast<aiTextureType>(type), 0, &filepath);

		//cout << filepath.C_Str() << endl;
		const char* lastSlash = strrchr(filepath.C_Str(), '\\');

		if (lastSlash) lastSlash++;  //'\\'가 있으면 한칸 이동, 없으면 filepath.C_Str()가 그대로 이름인 경우

		const char* fileName = lastSlash ? lastSlash : filepath.C_Str();

		char* fullTexturePath = new char[256];
		strcpy_s(fullTexturePath, 256, basePath);
		strcat_s(fullTexturePath, 256, fileName);

		//printf("%s : \n", fullTexturePath);
		return fullTexturePath;
	}
	else
	{
		//printf("%d : nullptr\n", static_cast<int>(type));
		return nullptr;
	}
}


ModelLoader::~ModelLoader()
{
}