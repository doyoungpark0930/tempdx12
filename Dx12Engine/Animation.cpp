#include "pch.h"


//나중에지워줘야..
#include <vector> 
#include <unordered_map>
#include <string>
#include <iostream>

#include "Bone.h"
#include "ModelLoader.h"
#include "Animation.h"

using namespace std;
void Animation::OnInit(maxNode* rootNode, ModelLoader* model, float duration)
{
	m_RootNode = rootNode;
	m_Duration = duration;
	m_TicksPerSecond = TicksPerSecond;
	m_BoneInfoMap = model->GetBoneInfoMap();
	m_Bones = new Bone[model->GetBoneCount()];

	InitBones(m_RootNode);
}


void Animation::InitBones(const maxNode* node)
{//일단 최적화 신경쓰지말고, animation대신 node를 넣어서 node순으로 쫙 bone생성하기

	int boneId = m_BoneInfoMap[node->name];
	m_Bones[boneId].OnInit(node->name, boneId, node);


	for (int i = 0; i < node->mNumChildren; i++)
		InitBones(node->mChildren[i]);

}



Animation::~Animation()
{
	SafeDeleteArray(&m_Bones);
}