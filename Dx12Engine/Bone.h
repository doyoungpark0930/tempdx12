#pragma once

struct aiNodeAnim;

struct KeyPosition
{
	Vector3 position;
	float timeStamp;
};

struct KeyRotation
{
	Quaternion orientation;
	float timeStamp;
};

struct KeyScale
{
	Vector3 scale;
	float timeStamp;
};

class Bone
{
public:
	/*reads keyframes from aiNodeAnim*/
	void OnInit(const std::string& name, int ID, const aiNodeAnim* channel, float *duration);

	~Bone();

	void Update(float animationTime);

	Matrix GetLocalTransform() { return m_LocalTransform; }
	std::string GetBoneName() const { return m_Name; }
	int GetBoneID() { return m_ID; }

	int GetPositionIndex(float animationTime);
	int GetRotationIndex(float animationTime);
	int GetScaleIndex(float animationTime);

private:
	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
	Matrix InterpolatePosition(float animationTime);
	Matrix InterpolateRotation(float animationTime);
	Matrix InterpolateScaling(float animationTime);

private:
	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;
	int m_NumPositions;
	int m_NumRotations;
	int m_NumScalings;

	Matrix m_LocalTransform;
	std::string m_Name;
	int m_ID;

};