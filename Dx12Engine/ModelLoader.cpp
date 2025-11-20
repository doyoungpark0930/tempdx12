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


void ModelLoader::Load(char* basePath, const char* filename)
{
	this->basePath = basePath;
	char fullPath[512];
	strcpy_s(fullPath, sizeof(fullPath), basePath);
	strcat_s(fullPath, sizeof(fullPath), filename);

	FILE* pStream = nullptr;
	fopen_s(&pStream, fullPath, "r");

	if (!pStream) {
		cout << "file can't be opened" << endl;
		return;
	}

	char line[512];

	//Indent
	int MATERIAL_LIST_Indent = 0;
	int MATERIAL_Indent = 0;
	int SUBMATERIAL_Indent = 0;
	int HELPER_indent = 0;
	int GEOM_indent = 0;
	int MESH_indent = 0;
	int FACE_indent = 0;
	int BONE_indent = 0;
	int TRANSFORM_indent = 0;
	int ANIM_Indent = 0;
	int POS_Indent = 0;
	int ROT_Indent = 0;
	int SCALE_Indent = 0;

	char* startPlace = nullptr; //문자열 시작 주소 파악

	//HelperObject Node만들 때 사용하는 포인터
	maxNode* curNode = nullptr;
	maxNode* beforeNode = nullptr;

	//이전의 material 몇개 할당되었는지
	int* prevMaterialNum = nullptr;
	int allocatedMaterialNum = 0; //mtl하나 할당될때마다 하나씩 센다

	int temp = 0;

	m_animations = new Animation[1];

	while (fgets_trim(line, sizeof(line), pStream)) {

		//materal, mesh, index개수 받기
		{
			startPlace = strstr(line, "usedMaterialCnt");
			if (startPlace != nullptr)
			{
				startPlace += strlen("usedMaterialCnt");
				int x;
				if (sscanf_s(startPlace, "%d", &x) == 1)
				{
					m_materialNum = x;
					m_materials = new material[m_materialNum];

				}
				//mesh
				fgets_trim(line, sizeof(line), pStream);
				startPlace = strstr(line, "meshCnt");
				startPlace += strlen("meshCnt");
				if (sscanf_s(startPlace, "%d", &x) == 1)
				{
					m_meshesNum = x;
					m_meshes = new mesh[m_meshesNum];
					for (int i = 0; i < m_materialNum; i++)
					{
						m_materials[i].meshNum = m_meshesNum;
						m_materials[i].index = new UINT * [m_meshesNum];
						m_materials[i].face_cnt = new UINT[m_meshesNum];
						memset(m_materials[i].face_cnt, 0, sizeof(UINT) * m_meshesNum);
					}
				}
				for (int i = 0; i < m_meshesNum; i++)
				{
					fgets_trim(line, sizeof(line), pStream);
					startPlace = strstr(line, "indexCnt");
					startPlace += strlen("indexCnt");
					if (sscanf_s(startPlace, "%d", &x) == 1)
					{
						m_meshes[i].verticesNum = x;
						m_meshes[i].vertices = new Vertex[x];
						for (int j = 0; j < m_materialNum; j++)
						{
							m_materials[j].index[i] = new UINT[x];
							memset(m_materials[j].index[i], 0, sizeof(UINT) * x);
						}
					}
				}
			}
		}

		//Read MaterialList
		if (checkTitle(line, "*MATERIAL_LIST", MATERIAL_LIST_Indent))
		{
			while (fgets_trim(line, sizeof(line), pStream))
			{
				startPlace = strstr(line, "*MATERIAL_COUNT");
				if (startPlace != nullptr) {
					startPlace += strlen("*MATERIAL_COUNT");
					if (sscanf_s(startPlace, "%d", &m_noSubMtlNum) == 1)
					{
						prevMaterialNum = new int[m_noSubMtlNum];
						memset(prevMaterialNum, 0, sizeof(int) * m_noSubMtlNum);
					}
					continue;

				}

				if (checkTitle(line, "*MATERIAL ID", MATERIAL_Indent))
				{
					startPlace = strstr(line, "*MATERIAL ID");
					if (startPlace != nullptr) {
						startPlace += strlen("*MATERIAL ID");
						int id;
						if (sscanf_s(startPlace, "%d", &id) == 1)
						{
							prevMaterialNum[id] = allocatedMaterialNum;
						}

					}

					while (fgets_trim(line, sizeof(line), pStream))
					{
						//SubMaterial이 있다면
						if (checkTitle(line, "*SUBMATERIAL", SUBMATERIAL_Indent))
						{
							while (fgets_trim(line, sizeof(line), pStream))
							{
								if ((startPlace = strstr(line, "*TEXTURE")) != nullptr)
								{
									ReadMapInfo(pStream, allocatedMaterialNum);
								}
								if (line[SUBMATERIAL_Indent] == '}')
								{
									allocatedMaterialNum++;
									break;
								}
							}
						}
						//SubMaterial없고 자체 texture라면
						else if ((startPlace = strstr(line, "*TEXTURE")) != nullptr)
						{
							ReadMapInfo(pStream, allocatedMaterialNum);
						}

						if (line[MATERIAL_Indent] == '}')
						{
							allocatedMaterialNum++;
							break;
						}
					}
				}

				if (line[MATERIAL_LIST_Indent] == '}')
				{
					break;
				}
			}
		}

		//HelperObject
		if (checkTitle(line, "*HELPEROBJECT", HELPER_indent))
		{
			m_BoneCounter++;
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
						if (beforeNode->mNumChildren > 20) __debugbreak(); //최대자식 수 넘어감. maxNode에서 설정
					}
					else
					{
						maxNode* parentNode = findNode(rootNode, parentName);
						if (!parentNode) __debugbreak();
						curNode->mParent = parentNode;

						int childNum = parentNode->mNumChildren;
						parentNode->mChildren[childNum] = curNode;
						parentNode->mNumChildren++;
						if (parentNode->mNumChildren > 20) __debugbreak(); //최대자식 수 넘어감. maxNode에서 설정
					}
					continue;
				}

				if (checkTitle(line, "*NODE_TRANSFORMATION_MATRIX", TRANSFORM_indent))
				{
					int offsetRow = 0;
					while (fgets_trim(line, sizeof(line), pStream))
					{
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

				startPlace = strstr(line, "*NODE_LOCAL_TRANSFORMATION_MATRIX_DECOMPOSITION");
				if (startPlace != nullptr)
				{
					fgets_trim(line, sizeof(line), pStream);
					startPlace = strstr(line, "*TM_TRANSLATION_COMPONENTS");
					if (startPlace != nullptr)
					{
						startPlace += strlen("*TM_TRANSLATION_COMPONENTS");
						float x[3];
						if (curNode && (sscanf_s(startPlace, "%f %f %f", &x[0], &x[1], &x[2]) == 3))
						{
							curNode->decomp_t.x = x[0];
							curNode->decomp_t.y = x[1];
							curNode->decomp_t.z = x[2];
						}
						else __debugbreak();
					}

					fgets_trim(line, sizeof(line), pStream);
					startPlace = strstr(line, "*TM_ESSENTIAL_ROTATION");
					if (startPlace != nullptr)
					{
						startPlace += strlen("*TM_ESSENTIAL_ROTATION");
						float x[4];
						if (curNode && (sscanf_s(startPlace, "%f %f %f %f", &x[0], &x[1], &x[2], &x[3]) == 4))
						{
							curNode->decomp_q.x = x[0];
							curNode->decomp_q.y = x[1];
							curNode->decomp_q.z = x[2];
							curNode->decomp_q.w = x[3];
						}
						else __debugbreak();
					}

					fgets_trim(line, sizeof(line), pStream);
					startPlace = strstr(line, "*TM_STRETCH_ROTATION");
					if (startPlace != nullptr)
					{
						startPlace += strlen("*TM_STRETCH_ROTATION");
						float x[4];
						if (curNode && (sscanf_s(startPlace, "%f %f %f %f", &x[0], &x[1], &x[2], &x[3]) == 4))
						{

							curNode->decomp_u.x = x[0];
							curNode->decomp_u.y = x[1];
							curNode->decomp_u.z = x[2];
							curNode->decomp_u.w = x[3];
						}
						else __debugbreak();
					}

					fgets_trim(line, sizeof(line), pStream);
					startPlace = strstr(line, "*TM_STRETCH_FACTORS");
					if (startPlace != nullptr)
					{
						startPlace += strlen("*TM_STRETCH_FACTORS");
						float x[3];
						if (curNode && (sscanf_s(startPlace, "%f %f %f", &x[0], &x[1], &x[2]) == 3))
						{
							curNode->decomp_k.x = x[0];
							curNode->decomp_k.y = x[1];
							curNode->decomp_k.z = x[2];
						}
						else __debugbreak();
					}

					fgets_trim(line, sizeof(line), pStream); //FLIP_SIGN
					continue;


				}

				if (checkTitle(line, "*TM_ANIMATION", ANIM_Indent))
				{
					while (fgets_trim(line, sizeof(line), pStream))
					{

						if (checkTitle(line, "*CONTROL_POS_TRACK", POS_Indent))
						{
							while (fgets_trim(line, sizeof(line), pStream))
							{
								startPlace = strstr(line, "*CONTROL_POS_SAMPLE");
								if (startPlace != nullptr)
								{
									startPlace += strlen("*CONTROL_POS_SAMPLE");
									float x[4];
									if (curNode && (sscanf_s(startPlace, "%f %f %f %f", &x[0], &x[1], &x[2], &x[3]) == 4))
									{
										curNode->mPositionKeysTime[curNode->mNumPositionKeys] = x[0];
										curNode->mPositionKeysValue[curNode->mNumPositionKeys].x = x[1];
										curNode->mPositionKeysValue[curNode->mNumPositionKeys].y = x[2];
										curNode->mPositionKeysValue[curNode->mNumPositionKeys].z = x[3];

										curNode->mNumPositionKeys++;
									}
									else __debugbreak();
								}

								if (line[POS_Indent] == '}')
								{
									break;
								}
							}
						}

						if (checkTitle(line, "*CONTROL_ROT_TRACK", ROT_Indent))
						{
							while (fgets_trim(line, sizeof(line), pStream))
							{
								startPlace = strstr(line, "*CONTROL_ROT_SAMPLE");
								if (startPlace != nullptr)
								{
									startPlace += strlen("*CONTROL_ROT_SAMPLE");
									float x[5];
									if (curNode && (sscanf_s(startPlace, "%f %f %f %f %f", &x[0], &x[1], &x[2], &x[3], &x[4]) == 5))
									{
										curNode->mRotationKeysTime[curNode->mNumRotationKeys] = x[0];
										curNode->mRotationKeysValue[curNode->mNumRotationKeys].x = x[1];
										curNode->mRotationKeysValue[curNode->mNumRotationKeys].y = x[2];
										curNode->mRotationKeysValue[curNode->mNumRotationKeys].z = x[3];
										curNode->mRotationKeysValue[curNode->mNumRotationKeys].w = x[4];

										curNode->mNumRotationKeys++;
									}
									else __debugbreak();
								}
								if (line[ROT_Indent] == '}')
								{
									break;
								}
							}
						}

						if (checkTitle(line, "*CONTROL_SCALE_TRACK", SCALE_Indent))
						{
							while (fgets_trim(line, sizeof(line), pStream))
							{
								startPlace = strstr(line, "*CONTROL_SCALE_SAMPLE");
								if (startPlace != nullptr)
								{
									startPlace += strlen("*CONTROL_SCALE_SAMPLE");
									float x[4];
									if (curNode && (sscanf_s(startPlace, "%f %f %f %f", &x[0], &x[1], &x[2], &x[3]) == 4))
									{
										curNode->mScaleKeysTime[curNode->mNumScaleKeys] = x[0];
										curNode->mScaleKeysValue[curNode->mNumScaleKeys].x = x[1];
										curNode->mScaleKeysValue[curNode->mNumScaleKeys].y = x[2];
										curNode->mScaleKeysValue[curNode->mNumScaleKeys].z = x[3];

										curNode->mNumScaleKeys++;

									}
									else __debugbreak();
								}

								if (line[SCALE_Indent] == '}')
								{
									break;
								}
							}
						}



						if (line[ANIM_Indent] == '}')
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
						if (checkTitle(line, "*MESH_FACE_LIST", FACE_indent))
						{
							int index = 0;
							int mtlID = 0;
							while (fgets_trim(line, sizeof(line), pStream))
							{
								if (line[FACE_indent] == '}')
								{
									break;
								}
								fgets_trim(line, sizeof(line), pStream); //mtl id
								int x[2];
								if (sscanf_s(line, "%d %d", &x[0], &x[1]) == 2)
								{
									if (prevMaterialNum != nullptr) {
										mtlID = prevMaterialNum[x[0]] + x[1];
									}

								}

								fgets_trim(line, sizeof(line), pStream); //vertex
								ParseVertexLine(line, &m_meshes[meshCnt].vertices[index], meshCnt, mtlID, index);
								fgets_trim(line, sizeof(line), pStream); //weight
								ParseBoneWeights(line, &m_meshes[meshCnt].vertices[index]);
								index++;

								fgets_trim(line, sizeof(line), pStream);//vertex
								ParseVertexLine(line, &m_meshes[meshCnt].vertices[index], meshCnt, mtlID, index);
								fgets_trim(line, sizeof(line), pStream);//weight
								ParseBoneWeights(line, &m_meshes[meshCnt].vertices[index]);
								index++;

								fgets_trim(line, sizeof(line), pStream);//vertex
								ParseVertexLine(line, &m_meshes[meshCnt].vertices[index], meshCnt, mtlID, index);
								fgets_trim(line, sizeof(line), pStream);//weight
								ParseBoneWeights(line, &m_meshes[meshCnt].vertices[index]);
								index++;

								m_materials[mtlID].face_cnt[meshCnt]++;
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
					meshCnt++;
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
				if (sscanf_s(line + boneName.size() + 3, "%d", &boneId) == 1)
				{
					m_BoneInfoMap[boneName] = boneId;
				}

				if (line[BONE_indent] == '}')
				{
					break;
				}
			}
		}

	}
	UpdateResourcePath(m_materials, m_materialNum);



	m_animations[0].OnInit(rootNode, this, 4000); //임시로 값 12000그냥 넣음
	//cout << m_materialNum << endl;
	//cout << m_meshesNum << endl;
	//printNode(rootNode, 0);

	//PrintAllMaterialTextures(m_materials, m_materialNum);
	if (prevMaterialNum) delete[] prevMaterialNum;
	fclose(pStream);
}


//Resource파일 경로로 바꿔줌
void ModelLoader::UpdateResourcePath(material* m_materials, UINT m_materialNum)
{
	for (UINT i = 0; i < m_materialNum; ++i)
	{
		m_materials[i].albedoTexFilename = UpdatePath(m_materials[i].albedoTexFilename);
		m_materials[i].aoTexFilename = UpdatePath(m_materials[i].aoTexFilename);
		m_materials[i].metallicTexFilename = UpdatePath(m_materials[i].metallicTexFilename);
		m_materials[i].normalTexFilename = UpdatePath(m_materials[i].normalTexFilename);
		m_materials[i].roughnessTexFilename = UpdatePath(m_materials[i].roughnessTexFilename);
	}
}

char* ModelLoader::UpdatePath(char* filePath)
{
	if (filePath != nullptr)
	{
		const char* lastSlash = strrchr(filePath, '\\');

		if (lastSlash) lastSlash++;  //'\\'가 있으면 한칸 이동, 없으면 filepath.C_Str()가 그대로 이름인 경우

		const char* fileName = lastSlash ? lastSlash : filePath;

		char* fullTexturePath = new char[256];
		strcpy_s(fullTexturePath, 256, basePath);
		strcat_s(fullTexturePath, 256, fileName);

		SafeDeleteArray(&filePath);
		return fullTexturePath;
	}
	else return nullptr;


}

maxNode* ModelLoader::findNode(maxNode* node, const std::string& name)
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



void ModelLoader::printNode(maxNode* node, int depth)
{
	if (!node)
		return;

	// 부모-자식 구조 들여쓰기
	for (int i = 0; i < depth; ++i)
		std::cout << "  ";

	std::cout << "└─ " << node->name << " (" << node->mNumChildren << " children)" << std::endl;

	// -----------------------------------------------------
   //  Decomposed Transform 출력
   // -----------------------------------------------------
	for (int i = 0; i < depth + 1; ++i)
		std::cout << "  ";
	std::cout << "Decomposed Transform:" << std::endl;

	for (int i = 0; i < depth + 2; ++i)
		std::cout << "  ";
	std::cout << "T = (" << node->decomp_t.x << ", " << node->decomp_t.y << ", " << node->decomp_t.z << ")\n";

	for (int i = 0; i < depth + 2; ++i)
		std::cout << "  ";
	std::cout << "Q = (" << node->decomp_q.x << ", " << node->decomp_q.y << ", "
		<< node->decomp_q.z << ", " << node->decomp_q.w << ")\n";

	for (int i = 0; i < depth + 2; ++i)
		std::cout << "  ";
	std::cout << "U = (" << node->decomp_u.x << ", " << node->decomp_u.y << ", "
		<< node->decomp_u.z << ", " << node->decomp_u.w << ")\n";

	for (int i = 0; i < depth + 2; ++i)
		std::cout << "  ";
	std::cout << "K = (" << node->decomp_k.x << ", " << node->decomp_k.y << ", " << node->decomp_k.z << ")\n";

	// -----------------------------------------------------
	//  Position Keys
	// -----------------------------------------------------
	if (node->mNumPositionKeys > 0)
	{
		for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
		std::cout << "Position Keys (" << node->mNumPositionKeys << "):\n";

		for (int k = 0; k < node->mNumPositionKeys; ++k)
		{
			for (int i = 0; i < depth + 2; ++i) std::cout << "  ";
			const Vector3& v = node->mPositionKeysValue[k];
			std::cout << "[" << k << "] time=" << node->mPositionKeysTime[k]
				<< "  value=(" << v.x << ", " << v.y << ", " << v.z << ")\n";
		}
	}

	// -----------------------------------------------------
	//  Rotation Keys
	// -----------------------------------------------------
	if (node->mNumRotationKeys > 0)
	{
		for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
		std::cout << "Rotation Keys (" << node->mNumRotationKeys << "):\n";

		for (int k = 0; k < node->mNumRotationKeys; ++k)
		{
			for (int i = 0; i < depth + 2; ++i) std::cout << "  ";
			const Vector4& q = node->mRotationKeysValue[k];
			std::cout << "[" << k << "] time=" << node->mRotationKeysTime[k]
				<< "  value=(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")\n";
		}
	}

	// -----------------------------------------------------
	//  Scale Keys
	// -----------------------------------------------------
	if (node->mNumScaleKeys > 0)
	{
		for (int i = 0; i < depth + 1; ++i) std::cout << "  ";
		std::cout << "Scale Keys (" << node->mNumScaleKeys << "):\n";

		for (int k = 0; k < node->mNumScaleKeys; ++k)
		{
			for (int i = 0; i < depth + 2; ++i) std::cout << "  ";
			const Vector3& s = node->mScaleKeysValue[k];
			std::cout << "[" << k << "] time=" << node->mScaleKeysTime[k]
				<< "  value=(" << s.x << ", " << s.y << ", " << s.z << ")\n";
		}
	}


	// 자식 노드 재귀 출력
	for (int i = 0; i < node->mNumChildren; ++i)
	{
		printNode(node->mChildren[i], depth + 1);
	}
}


inline void ModelLoader::ParseVertexLine(const char* startPlace, Vertex* vertex, int meshCnt, int mtlID, int index)
{
	float x[11];
	//position, uv, normal, tangent순
	if (sscanf_s(startPlace, "%f %f %f %f %f %f %f %f %f %f %f",
		&x[0], &x[1], &x[2], &x[3], &x[4],
		&x[5], &x[6], &x[7], &x[8], &x[9], &x[10]) == 11)
	{
		if (vertex != nullptr)
		{
			vertex->Pos = { x[0], x[1], x[2] };
			vertex->Tex = { x[3], x[4] };
			vertex->Normal = { x[5], x[6], x[7] };
			vertex->tangent = { x[8], x[9], x[10] };
		}
		else __debugbreak();
	}

	int face_cnt = m_materials[mtlID].face_cnt[meshCnt];
	m_materials[mtlID].index[meshCnt][face_cnt * 3 + index % 3] = index;
}


inline void ModelLoader::ParseBoneWeights(char* line, Vertex* vertex)
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
				if (vertex != nullptr)
				{
					vertex->m_Weights[i] = weight;
					vertex->m_BoneIDs[i] = static_cast<uint8_t>(boneID);
				}
				else __debugbreak();
			}

			startPlace = findNthNumberStart(startPlace, 3); // 다음 float,int 쌍
		}
	}
}

void ModelLoader::InsertMtlValue(int allocatedMaterialNum, std::string& mapName, std::string& mapPath)
{
	char* albedoTexFilename = nullptr;
	char* aoTexFilename = nullptr;
	char* normalTexFilename = nullptr;
	char* metallicTexFilename = nullptr;
	char* roughnessTexFilename = nullptr;
	if (mapName == "base_color_texture")
	{
		m_materials[allocatedMaterialNum].albedoTexFilename = new char[mapPath.length() + 1];
		strcpy_s(m_materials[allocatedMaterialNum].albedoTexFilename, mapPath.length() + 1, mapPath.c_str());
	}
	else if (mapName == "ao_texture")
	{
		m_materials[allocatedMaterialNum].aoTexFilename = new char[mapPath.length() + 1];
		strcpy_s(m_materials[allocatedMaterialNum].aoTexFilename, mapPath.length() + 1, mapPath.c_str());
	}
	else if (mapName == "normalmap_texture")
	{
		m_materials[allocatedMaterialNum].normalTexFilename = new char[mapPath.length() + 1];
		strcpy_s(m_materials[allocatedMaterialNum].normalTexFilename, mapPath.length() + 1, mapPath.c_str());
	}
	else if (mapName == "metallic_texture")
	{
		m_materials[allocatedMaterialNum].metallicTexFilename = new char[mapPath.length() + 1];
		strcpy_s(m_materials[allocatedMaterialNum].metallicTexFilename, mapPath.length() + 1, mapPath.c_str());
	}
	else if (mapName == "roughness_texture")
	{
		m_materials[allocatedMaterialNum].roughnessTexFilename = new char[mapPath.length() + 1];
		strcpy_s(m_materials[allocatedMaterialNum].roughnessTexFilename, mapPath.length() + 1, mapPath.c_str());
	}
}

void ModelLoader::PrintAllMaterialTextures(material* m_materials, UINT m_materialNum)
{
	for (UINT i = 0; i < m_materialNum; ++i)
	{
		std::cout << "===== Material " << i << " =====" << std::endl;

		if (m_materials[i].albedoTexFilename)
			std::cout << "Albedo:    " << m_materials[i].albedoTexFilename << std::endl;
		else
			std::cout << "Albedo:    (null)" << std::endl;

		if (m_materials[i].aoTexFilename)
			std::cout << "AO:        " << m_materials[i].aoTexFilename << std::endl;
		else
			std::cout << "AO:        (null)" << std::endl;

		if (m_materials[i].normalTexFilename)
			std::cout << "Normal:    " << m_materials[i].normalTexFilename << std::endl;
		else
			std::cout << "Normal:    (null)" << std::endl;

		if (m_materials[i].metallicTexFilename)
			std::cout << "Metallic:  " << m_materials[i].metallicTexFilename << std::endl;
		else
			std::cout << "Metallic:  (null)" << std::endl;

		if (m_materials[i].roughnessTexFilename)
			std::cout << "Roughness: " << m_materials[i].roughnessTexFilename << std::endl;
		else
			std::cout << "Roughness: (null)" << std::endl;

		std::cout << std::endl;
	}
}

void ModelLoader::ReadMapInfo(FILE* pStream, int allocatedMaterialNum)
{
	char line[512];
	char* startPlace = nullptr;

	std::string mapName;
	std::string mapPath;

	// MAP_NAME 읽기
	fgets_trim(line, sizeof(line), pStream);
	startPlace = strstr(line, "*MAP_NAME");
	if (startPlace)
		FindStr(startPlace, mapName);

	// normalmap_texture의 경우 
	if (mapName == "normalmap_texture")
	{
		fgets_trim(line, sizeof(line), pStream);

		if (strstr(line, "*TEXTURE")) //*Texture인경우 두줄스킵,아닌경우 그냥 지나감
		{
			fgets_trim(line, sizeof(line), pStream);
			fgets_trim(line, sizeof(line), pStream);
		}
	}
	else
	{
		fgets_trim(line, sizeof(line), pStream);
	}

	// BITMAP 읽기
	startPlace = strstr(line, "*BITMAP");
	if (startPlace)
		FindStr(startPlace, mapPath);

	// 재질에 삽입
	InsertMtlValue(allocatedMaterialNum, mapName, mapPath);
}

ModelLoader::~ModelLoader()
{

}