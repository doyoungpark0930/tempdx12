#include "pch.h"
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
	m_BoneCounter = model->GetBoneCount();
	m_Bones = new Bone[m_BoneCounter];

	InitBones(m_RootNode);
}


void Animation::InitBones(const maxNode* node)
{//일단 최적화 신경쓰지말고, animation대신 node를 넣어서 node순으로 쫙 bone생성하기

	if (m_BoneInfoMap.find(node->name) != m_BoneInfoMap.end())
	{
		int boneId = m_BoneInfoMap[node->name];
		m_Bones[boneId].OnInit(node->name, boneId, node);
	}
	else //bone에 해당하지 않는 첫 rootNode인 경우
	{
		int boneId = m_BoneCounter - 1; //맨 마지막 index를 rootNode로
		m_Bones[boneId].OnInit(node->name, boneId, node);
	}


	for (int i = 0; i < node->mNumChildren; i++)
		InitBones(node->mChildren[i]);

}



Animation::~Animation()
{
	SafeDeleteArray(&m_Bones);
}