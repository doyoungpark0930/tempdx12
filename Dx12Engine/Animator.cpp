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
	m_FinalBoneMatrices = new Matrix[ModelMatrixNum];

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
}

void Animator::CalculateBoneTransform(const maxNode* node, Matrix parentTransform)
{
	std::string nodeName = node->name;

	Matrix nodeTransform;
	Matrix globalTransformation;
	auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	Bone* Bones = m_CurrentAnimation->GetBone();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int boneIndex = boneInfoMap[nodeName];
		Bones[boneIndex].Update(m_CurrentTime);
		//Bones[boneIndex].Update(200);
		nodeTransform = Bones[boneIndex].GetLocalTransform();

		globalTransformation = nodeTransform * parentTransform;
		Matrix meshWorld =
		{
			0.3937, 0.0000,  0.0000,  0.0000,
			0.0000,-0.0000, -0.3937,  0.0000,
			0.0000, 0.3937, -0.0000,  0.0000,
			0.0000, 0.0000,  0.0000,  1.0000
		};
		Matrix offset = meshWorld * node->offset;
		m_FinalBoneMatrices[boneIndex] = (m_defaultTransform.Invert() * offset* globalTransformation * m_defaultTransform).Transpose();

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