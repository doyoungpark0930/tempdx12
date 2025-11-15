#pragma once

class ModelLoader;
class Animation;
struct AssimpNodeData;

class Animator
{
public:
	void OnInit(Animation* Animation, Matrix defaultTransform);

	void UpdateAnimation(float dt);

	void PlayAnimation(Animation* pAnimation);

	void CalculateBoneTransform(const maxNode* node, Matrix parentTransform);
	int testCnt = 0;

	Matrix* GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
	UINT matricesNum = 64;
	~Animator();

private:
	Matrix* m_FinalBoneMatrices = nullptr;
	Animation* m_CurrentAnimation = nullptr;
	Matrix m_defaultTransform;
	float m_CurrentTime;
	float m_DeltaTime;
};