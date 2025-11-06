#include "pch.h"
#include "ModelLoader.h"
#include "GeometryGenerator.h"

using namespace DirectX;

MeshDataInfo GeometryGenerator::testReadFromFile(char* basePath, const char* filename)
{
	ModelLoader modelLoader;
	modelLoader.testLoad(basePath, filename);
	MeshDataInfo result;
	//result.meshes = modelLoader.meshData;
	result.rootNode = modelLoader.rootNode;
	Normalize(Vector3(0.0f), 1.0f, result, modelLoader);

	return result;

}

MeshDataInfo GeometryGenerator::ReadFromFile(char* basePath, const char* filename) {


	ModelLoader modelLoader;

	MeshDataInfo result;
	Normalize(Vector3(0.0f), 1.0f, result, modelLoader);

	return result;
}

MeshDataInfo GeometryGenerator::AnimationReadFromFile(char* basePath, const char* filename, const char** animationNames, UINT animationNum) {


	ModelLoader modelLoader;

	MeshDataInfo result;
	result.m_animations = modelLoader.m_animations;
	result.animationCnt = animationNum;
	Normalize(Vector3(0.0f, 0.0f, 0.0f), 1.0f, result, modelLoader);

	return result;
}

void GeometryGenerator::Normalize(const Vector3 center,
	const float longestLength,
	MeshDataInfo& result, ModelLoader& modelLoader)
{

	// Normalize vertices
	Vector3 vmin(1000, 1000, 1000);
	Vector3 vmax(-1000, -1000, -1000);
	/*for (int i = 0; i < result.meshCount; i++) {
		for (int j = 0; j < result.meshes[i].verticesNum; j++) {
			vmin.x = XMMin(vmin.x, result.meshes[i].vertices[j].Pos.x);
			vmin.y = XMMin(vmin.y, result.meshes[i].vertices[j].Pos.y);
			vmin.z = XMMin(vmin.z, result.meshes[i].vertices[j].Pos.z);
			vmax.x = XMMax(vmax.x, result.meshes[i].vertices[j].Pos.x);
			vmax.y = XMMax(vmax.y, result.meshes[i].vertices[j].Pos.y);
			vmax.z = XMMax(vmax.z, result.meshes[i].vertices[j].Pos.z);
		}
	}*/

	float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float scale = longestLength / XMMax(XMMax(dx, dy), dz);
	Vector3 translation = -(vmin + vmax) * 0.5f + center; //원점으로 옮긴 후 + center

	/*for (int i = 0; i < result.meshCount; i++) {
		for (int j = 0; j < result.meshes[i].verticesNum; j++) {
			result.meshes[i].vertices[j].Pos = (result.meshes[i].vertices[j].Pos + translation) * scale;
		}
	}*/
	
	modelLoader.defaultTransform =  Matrix::CreateTranslation(translation)* Matrix::CreateScale(scale);
	result.m_defaultTransform = modelLoader.defaultTransform;
}