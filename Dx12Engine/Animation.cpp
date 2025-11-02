#include "pch.h"

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

//나중에지워줘야..
#include <vector> 
#include <unordered_map>
#include <string>
#include <iostream>

#include "Bone.h"
#include "ModelLoader.h"
#include "Animation.h"

using namespace std;
void Animation::OnInit(const std::string& animationPath, ModelLoader* model)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	//	| aiProcess_OptimizeGraph); animation translation미리 계산

	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	m_Duration = animation->mDuration;
	//cout << m_Duration << endl;
	m_TicksPerSecond = animation->mTicksPerSecond;
	ReadMissingBones(animation, *model);
	ReadHeirarchyData(m_RootNode, scene->mRootNode, "zero");
}


//#include <iostream>
void Animation::ReadMissingBones(const aiAnimation* animation, ModelLoader& model)
{
	int size = animation->mNumChannels;

	auto& boneInfoMap = model.GetBoneInfoMap();
	int& boneCount = model.GetBoneCount();
	//printf("BeforeReadMissingBone : %d\n", boneInfoMap.size());
	m_Bones.reserve(boneCount + 10);
	m_Bones.resize(boneCount);
	//std::cout << animation->mName.C_Str() << std::endl;
	//printf("size : %d\n", size);
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i]; //한 채널은 하나의 뼈를 뜻함
		std::string boneName = channel->mNodeName.data;
		//std::cout << channel->mNodeName.data << std::endl;
		size_t pos = boneName.find("_$AssimpFbx$"); 
		if (pos != std::string::npos)
		{
			boneName = boneName.substr(0, pos); // _$AssimpFbx$이전 앞부분만 남김

		}

		if (boneInfoMap.find(boneName) == boneInfoMap.end()) //weight는 없어도, nodeTransform을 바꿔 자식 bone들에 영향을 미칠 수 있으므로
		{
			std::cout << "Bone not included in meshes was detected: " << boneName << std::endl;
			//boneInfoMap[boneName].id = boneCount;
			//boneCount++;
			//m_Bones.resize(boneCount);
		}
		else m_Bones[boneInfoMap[boneName].id].OnInit(channel->mNodeName.data, boneInfoMap[boneName].id, channel, &m_Duration);
	}
	//printf("AfterReadMissingBone : %d\n", boneInfoMap.size());

	m_BoneInfoMap = boneInfoMap;
}

//끝의 matrix와 끝의 aiNode
aiNode* Animation::FindBone(aiNode* src)
{
	if (m_BoneInfoMap.find(src->mName.data) == m_BoneInfoMap.end())
	{
		if (src->mNumChildren > 0) return FindBone(src->mChildren[0]);
		else return nullptr;
	}
	else
	{
		return src;
	}

}

Matrix Animation::CalculatePivotMatrix(aiNode* src, Matrix parentMatrix)
{
	Matrix nodeTransform;
	aiMatrix4x4& m = src->mTransformation;
	nodeTransform = Matrix(
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	).Transpose();

	nodeTransform = nodeTransform * parentMatrix;
	if (m_BoneInfoMap.find(src->mName.data) == m_BoneInfoMap.end())
	{
		if (src->mNumChildren > 0) return CalculatePivotMatrix(src->mChildren[0], nodeTransform);
		else __debugbreak();

	}
	else
	{
		return parentMatrix;
	}
}

void Animation::ReadHeirarchyData(AssimpNodeData& dest, aiNode* src, string parentName)
{
	assert(src);
	dest.name = src->mName.data;
	//std::cout << "parent : " << parentName << " -> " << src->mName.data << " : " << src->mNumChildren << std::endl;
	//std::cout << src->mName.data << test++ << std::endl;
	aiMatrix4x4& m = src->mTransformation; //node의 localTransform임
	dest.transformation = Matrix(
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	).Transpose();
	/*for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%8.4f ", dest.transformation.m[i][j]);
		}
		printf("\n");
	}*/
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		if (m_BoneInfoMap.find(src->mChildren[i]->mName.data) != m_BoneInfoMap.end())
		{
			ReadHeirarchyData(newData, src->mChildren[i], src->mName.data);
			newData.transformation = Matrix(); //처음부터 bone인 node는 Rotation * PreRotation * Translation가 Bones.Getlocaltransform에 이미 다 속해있어서 단위행렬로 넘겨줌
			dest.children.push_back(newData);
		}
		else //Bone이 아니고, Translation , PreRotation, Rotation 중 하나라면
		{
			auto childNode = FindBone(src->mChildren[i]); //Bone까지 추적
			if (childNode)
			{
				ReadHeirarchyData(newData, childNode, src->mName.data);
				newData.transformation = CalculatePivotMatrix(src->mChildren[i], Matrix()); //Rotation * PreRotation * Translation
				dest.children.push_back(newData);
			}
			else
			{
				dest.childrenCount--; //Bone이 아닌 node는 넣지 않기 때문
			}
		}
	}
}

Animation::~Animation()
{
}