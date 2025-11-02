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
		CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), Matrix());
	}
	testCnt = 0;
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	m_CurrentAnimation = pAnimation;
	m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, Matrix parentTransform)
{
	std::string nodeName = node->name;

	Matrix nodeTransform = node->transformation; //얘는 localTransform임
	Matrix ro_pre_tr = node->transformation;
	Matrix globalTransformation = nodeTransform * parentTransform; //parent는 worldTransform이자, localTransform들의 계산합
	auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	auto& Bones = m_CurrentAnimation->GetBone();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		//std::cout << "bone : " <<testCnt++<< nodeName << std::endl;
		//std::cout << "currentTime : " << m_CurrentTime << endl;
		Bones[boneInfoMap[nodeName].id].Update(m_CurrentTime);
		nodeTransform = Bones[boneInfoMap[nodeName].id].GetLocalTransform();

		//node->transformation은 Rotation * PreRotation * Translation미리 계산한 것
		globalTransformation = nodeTransform * ro_pre_tr * parentTransform;
		//globalTransformation = nodeTransform * parentTransform;
		int index = boneInfoMap[nodeName].id;
		Matrix offset = boneInfoMap[nodeName].offset;
		m_FinalBoneMatrices[index] = (m_defaultTransform.Invert() * offset * globalTransformation * m_defaultTransform).Transpose();
		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}
	else //RootNode. 다른 node라면 ReadHeirarchyData에서 실수한 것
	{
		auto& m = nodeTransform.m; 
		//std::cout <<"not bone : " << testCnt++ << nodeName << std::endl;
		
		/*for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				printf("%8.4f ", m[i][j]);
			}
			printf("\n");
		}*/

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], Matrix());

	}
}
Animator::~Animator()
{
	SafeDeleteArray(&m_FinalBoneMatrices);
}