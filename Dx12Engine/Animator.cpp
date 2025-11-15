#include "pch.h"
#include "ModelLoader.h"
#include "Animation.h"
#include "DXHelper.h"
#include "Animator.h"

#include <iostream>
using namespace std;
void Animator::OnInit(Animation* Animation, Matrix defaultTransform)
{
	m_CurrentTime = 0.0;
	m_CurrentAnimation = Animation;
	m_defaultTransform = defaultTransform;
	m_FinalBoneMatrices = new Matrix[matricesNum];
	auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();

}

void Animator::UpdateAnimation(float dt)
{
	m_DeltaTime = dt;
	if (m_CurrentAnimation)
	{
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		CalculateBoneTransform(m_CurrentAnimation->GetRootNode(), Matrix());
	}
	testCnt = 0;
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const maxNode* node, Matrix parentTransform)
{
	std::string nodeName = node->name;

	Matrix nodeTransform;
	Matrix globalTransformation;
	auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	auto& Bones = m_CurrentAnimation->GetBone();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		Bones[boneInfoMap[nodeName].id].Update(m_CurrentTime);
		nodeTransform = Bones[boneInfoMap[nodeName].id].GetLocalTransform();

		globalTransformation = nodeTransform * parentTransform;

		int index = boneInfoMap[nodeName].id;
		Matrix offset = node->offset;
		m_FinalBoneMatrices[index] = (m_defaultTransform.Invert() * offset * globalTransformation * m_defaultTransform).Transpose();
		for (int i = 0; i < node->mNumChildren; i++)
			CalculateBoneTransform(node->mChildren[i], globalTransformation);
	}
	else //RootNode. 다른 node라면 ReadHeirarchyData에서 실수한 것
	{
		for (int i = 0; i < node->mNumChildren; i++)
			CalculateBoneTransform(node->mChildren[i], Matrix());

	}
}
Animator::~Animator()
{
	SafeDeleteArray(&m_FinalBoneMatrices);
}