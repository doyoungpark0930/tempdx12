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
void Animation::OnInit(maxNode* rootNode, ModelLoader* model)
{
	m_RootNode = rootNode;
	//m_Duration = animation->mDuration;
	//cout << m_Duration << endl;
	m_TicksPerSecond = TickPerSecond;
	m_BoneInfoMap = model->GetBoneInfoMap();
	m_Bones.resize(model->GetBoneCount());
	InitBones(m_RootNode, *model);
}


void Animation::InitBones(const maxNode* node, ModelLoader& model)
{//일단 최적화 신경쓰지말고, animation대신 node를 넣어서 node순으로 쫙 bone생성하기

	auto& boneInfoMap = model.GetBoneInfoMap();

	m_Bones[boneInfoMap[node->name].id].OnInit(node->name, boneInfoMap[node->name].id, node, &m_Duration);


	for (int i = 0; i < node->mNumChildren; i++)
		InitBones(node->mChildren[i], model);

}



Animation::~Animation()
{
}