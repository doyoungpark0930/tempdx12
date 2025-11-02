#include "pch.h"
#ifdef new
#undef new  
#endif

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


#pragma comment(lib, "assimp-vc143-mtd.lib") 


//메모리 릭 확인 재정의
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "Bone.h"
using namespace DirectX::SimpleMath;

void Bone::OnInit(const std::string& name, int ID, const aiNodeAnim* channel, float* duration)

{
	m_Name = name;
	m_ID = ID;
	m_LocalTransform = Matrix();
	m_NumPositions = channel->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
	{
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		float timeStamp = channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data;
		data.position = Vector3(aiPosition.x, aiPosition.y, aiPosition.z);
		data.timeStamp = timeStamp;
		m_Positions.push_back(data);
	}

	m_NumRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
	{
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
		KeyRotation data;
		data.orientation = Quaternion(aiOrientation.x, aiOrientation.y, aiOrientation.z, aiOrientation.w);
		data.timeStamp = timeStamp;
		m_Rotations.push_back(data);
	}


	m_NumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
	{
		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		float timeStamp = channel->mScalingKeys[keyIndex].mTime;
		KeyScale data;
		data.scale = Vector3(scale.x, scale.y, scale.z);
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