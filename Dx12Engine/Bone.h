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
	void OnInit(const std::string& name, UINT ID, const maxNode* channel);

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
	KeyPosition m_Positions[30];
	KeyRotation m_Rotations[30];
	KeyScale m_Scales[30];
	int m_NumPositions;
	int m_NumRotations;
	int m_NumScalings;

	//max decomposition ±¸Á¶
	//https://help.autodesk.com/cloudhelp/2025/ENU/MAXDEV-CPP-API-REF/decomp_8h.html#ac27bcbc360b30936bacc4e1e7c7f7e5c
	Matrix m_ptm;
	Matrix m_rtm;
	Matrix m_srtm;
	Matrix m_srtmInv;
	Matrix m_stm;

	Matrix m_LocalTransform;
	std::string m_Name;
	int m_ID;

};