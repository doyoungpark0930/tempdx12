#include "pch.h"

#include "Bone.h"
using namespace DirectX::SimpleMath;

void Bone::OnInit(const std::string& name, UINT ID, const maxNode* node)
{
	m_Name = name;
	m_ID = ID;
	m_LocalTransform = Matrix();


	Vector3 rtmAxis(node->decomp_q.x, node->decomp_q.y, node->decomp_q.z);
	float rtmAngle = node->decomp_q.w;
	Quaternion rtmQ = Quaternion::CreateFromAxisAngle(rtmAxis, rtmAngle);
	rtmQ.Normalize();

	Vector3 srtmAxis(node->decomp_u.x, node->decomp_u.y, node->decomp_u.z);
	float srtmAngle = node->decomp_u.w;
	Quaternion srtmQ = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);

	if (srtmAxis.LengthSquared() > 1e-6f)
	{
		srtmQ = Quaternion::CreateFromAxisAngle(srtmAxis, srtmAngle);
		srtmQ.Normalize();
	}

	m_ptm = Matrix::CreateTranslation(node->decomp_t);
	m_rtm = Matrix::CreateFromQuaternion(rtmQ);
	m_srtm = Matrix::CreateFromQuaternion(srtmQ);
	m_srtmInv = m_srtm.Invert();
	m_stm = Matrix::CreateScale(node->decomp_k);

	m_NumPositions = node->mNumPositionKeys;

	for (int positionIndex = 0; positionIndex < m_NumPositions; positionIndex++)
	{
		m_Positions[positionIndex].position = node->mPositionKeysValue[positionIndex];
		m_Positions[positionIndex].timeStamp = node->mPositionKeysTime[positionIndex];
	}

	m_NumRotations = node->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < m_NumRotations; rotationIndex++)
	{
		Vector3 axis(node->mRotationKeysValue[rotationIndex].x,
			node->mRotationKeysValue[rotationIndex].y,
			node->mRotationKeysValue[rotationIndex].z);
		float angle = node->mRotationKeysValue[rotationIndex].w;

		Quaternion Orientation = Quaternion::CreateFromAxisAngle(axis, angle);
		Orientation.Normalize();

		m_Rotations[rotationIndex].orientation = Orientation;
		m_Rotations[rotationIndex].timeStamp = node->mRotationKeysTime[rotationIndex];
	}


	m_NumScalings = node->mNumScaleKeys;
	for (int keyIndex = 0; keyIndex < m_NumScalings; keyIndex++)
	{
		Vector3 scale = node->mScaleKeysValue[keyIndex];
		float timeStamp = node->mScaleKeysTime[keyIndex];
		m_Scales[keyIndex].scale = scale;
		m_Scales[keyIndex].timeStamp = timeStamp;
	}

}

void Bone::Update(float animationTime)
{
	Matrix ptm = InterpolatePosition(animationTime);
	Matrix rtm = InterpolateRotation(animationTime);
	Matrix stm = InterpolateScaling(animationTime);

	m_LocalTransform = m_srtmInv * stm * m_srtm * rtm * ptm;
		//m_srtmInv * m_stm * m_srtm * m_rtm * m_ptm;
		//m_srtmInv * stm * m_srtm * rtm * ptm;
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
	if (m_NumPositions < 2)
		return m_ptm;

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
	if (m_NumRotations < 2) 
	{
		return m_rtm;
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
	if (m_NumScalings < 2)
		return m_stm;

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