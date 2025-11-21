#pragma once

class ModelLoader;
class Animation;

class Animator
{
public:
	void OnInit(Animation* Animation, Matrix defaultTransform);

	void UpdateAnimation(float dt);

	void CalculateBoneTransform(const maxNode* node, Matrix parentTransform);

	Matrix* GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
	~Animator();

private:
	Matrix* m_FinalBoneMatrices = nullptr;
	Animation* m_CurrentAnimation = nullptr;
	Matrix m_defaultTransform;
	float m_CurrentTime;
	float m_DeltaTime;
};