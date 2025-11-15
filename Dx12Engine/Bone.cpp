#include "pch.h"

#include "Bone.h"
using namespace DirectX::SimpleMath;

void Bone::OnInit(const std::string& name, int ID, const maxNode* node, float* duration)
{
	m_Name = name;
	m_ID = ID;
	m_LocalTransform = Matrix();
	m_NumPositions = node->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
	{
		Vector3 Position = node->mPositionKeysValue[positionIndex];
		float timeStamp = node->mPositionKeysTime[positionIndex];
		KeyPosition data;
		data.position = Position;
		data.timeStamp = timeStamp;
		m_Positions.push_back(data);
	}

	m_NumRotations = node->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
	{
		Quaternion Orientation = node->mRotationKeysValue[rotationIndex];
		float timeStamp = node->mRotationKeysTime[rotationIndex];
		KeyRotation data;
		data.orientation = Orientation;
		data.timeStamp = timeStamp;
		m_Rotations.push_back(data);
	}


	m_NumScalings = node->mNumScaleKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
	{
		Vector3 scale = node->mScaleKeysValue[keyIndex];
		float timeStamp = node->mScaleKeysTime[keyIndex];
		KeyScale data;
		data.scale = scale;
		data.timeStamp = timeStamp;
		m_Scales.push_back(data);
	}

}

void Bone::Update(float animationTime)
{
	Matrix translation = InterpolatePosition(animationTime);
	Matrix rotation = InterpolateRotation(animationTime);
	Matrix scale = InterpolateScaling(animationTime);
	m_LocalTransform = scale * rotation * translation;
}

/* Gets the current index on mKeyPositions to interpolate to based on
	the current animation time*/
int Bone::GetPositionIndex(float animationTime)
{
	for (int index = 0; index < m_NumPositions - 1; ++index)
	{
		if (animationTime < m_Positions[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

/* Gets the current index on mKeyRotations to interpolate to based on the
current animation time*/
int Bone::GetRotationIndex(float animationTime)
{
	for (int index = 0; index < m_NumRotations - 1; ++index)
	{
		if (animationTime < m_Rotations[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

/* Gets the current index on mKeyScalings to interpolate to based on the
current animation time */
int Bone::GetScaleIndex(float animationTime)
{
	for (int index = 0; index < m_NumScalings - 1; ++index)
	{
		if (animationTime < m_Scales[index + 1].timeStamp)
			return index;
	}
	assert(0);
}

/* Gets normalized value for Lerp & Slerp*/
float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}

/*figures out which position keys to interpolate b/w and performs the interpolation
and returns the translation matrix*/
Matrix Bone::InterpolatePosition(float animationTime)
{
	if (1 == m_NumPositions)
		return Matrix::CreateTranslation(m_Positions[0].position);

	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
		m_Positions[p1Index].timeStamp, animationTime);

	Vector3 finalPosition = Vector3::Lerp(
		m_Positions[p0Index].position,
		m_Positions[p1Index].position,
		scaleFactor
	);
	return Matrix::CreateTranslation(finalPosition);
}

/*figures out which rotations keys to interpolate b/w and performs the interpolation
and returns the rotation matrix*/
Matrix Bone::InterpolateRotation(float animationTime)
{
	if (1 == m_NumRotations)
	{
		Quaternion rotation = m_Rotations[0].orientation;
		rotation.Normalize();
		return Matrix::CreateFromQuaternion(rotation);
	}

	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
		m_Rotations[p1Index].timeStamp, animationTime);
	Quaternion finalRotation = Quaternion::Slerp(
		m_Rotations[p0Index].orientation,
		m_Rotations[p1Index].orientation,
		scaleFactor
	);
	finalRotation.Normalize();
	return Matrix::CreateFromQuaternion(finalRotation);
}

/*figures out which scaling keys to interpolate b/w and performs the interpolation
and returns the scale matrix*/
Matrix Bone::InterpolateScaling(float animationTime)
{
	if (1 == m_NumScalings)
		return Matrix::CreateScale(m_Scales[0].scale);

	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;
	float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
		m_Scales[p1Index].timeStamp, animationTime);
	Vector3  finalScale = Vector3::Lerp(
		m_Scales[p0Index].scale,
		m_Scales[p1Index].scale,
		scaleFactor
	);
	return Matrix::CreateScale(finalScale);
}

Bone::~Bone()
{

}