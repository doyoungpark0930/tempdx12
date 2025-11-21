#include "pch.h"
#include "ModelLoader.h"
#include "Animation.h"
#include "DXHelper.h"
#include "Bone.h"
#include "Animator.h"


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
		
		nodeTransform = Bones[boneIndex].GetLocalTransform();

		globalTransformation = nodeTransform * parentTransform;
		Matrix Inv = globalTransformation.Invert();

		Matrix offset = node->offset;
		m_FinalBoneMatrices[boneIndex] = (m_defaultTransform.Invert() * offset * globalTransformation * m_defaultTransform).Transpose();

		for (int i = 0; i < node->mNumChildren; i++)
			CalculateBoneTransform(node->mChildren[i], globalTransformation);
	}
	else //boneÀÌ ¾Æ´Ñ rootnode. 
	{
		int boneIndex = m_CurrentAnimation->GetBoneCount() - 1;
		Bones[boneIndex].Update(m_CurrentTime);
		nodeTransform = Bones[boneIndex].GetLocalTransform();

		globalTransformation = nodeTransform * Matrix();
		CalculateBoneTransform(node->mChildren[0], globalTransformation);
	}

}
Animator::~Animator()
{
	SafeDeleteArray(&m_FinalBoneMatrices);
}