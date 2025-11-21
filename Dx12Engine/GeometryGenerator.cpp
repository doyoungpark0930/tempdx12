#include "pch.h"
#include "ModelLoader.h"
#include "GeometryGenerator.h"

using namespace DirectX;

MeshDataInfo GeometryGenerator::ReadFromFile(char* basePath, const char* filename)
{
	ModelLoader modelLoader;
	modelLoader.Load(basePath, filename);
	MeshDataInfo result;
	result.Materials = modelLoader.m_materials;
	result.materialNum = modelLoader.m_materialNum;
	result.meshes = modelLoader.m_meshes;
	result.meshNum = modelLoader.m_meshesNum;
	result.rootNode = modelLoader.rootNode;
	result.m_animations = modelLoader.m_animations;
	Normalize(Vector3(0.0f), 1.0f, result, modelLoader);

	return result;

}
void GeometryGenerator::Normalize(const Vector3 center,
	const float longestLength,
	MeshDataInfo& result, ModelLoader& modelLoader)
{

	// Normalize vertices
	Vector3 vmin(1000, 1000, 1000);
	Vector3 vmax(-1000, -1000, -1000);

	for (int i = 0; i < result.meshNum; i++) {
		for (int j = 0; j < result.meshes[i].verticesNum; j++) {
			vmin.x = XMMin(vmin.x, result.meshes[i].vertices[j].Pos.x);
			vmin.y = XMMin(vmin.y, result.meshes[i].vertices[j].Pos.y);
			vmin.z = XMMin(vmin.z, result.meshes[i].vertices[j].Pos.z);
			vmax.x = XMMax(vmax.x, result.meshes[i].vertices[j].Pos.x);
			vmax.y = XMMax(vmax.y, result.meshes[i].vertices[j].Pos.y);
			vmax.z = XMMax(vmax.z, result.meshes[i].vertices[j].Pos.z);
		}
	}

	float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
	float scale = longestLength / XMMax(XMMax(dx, dy), dz);
	Vector3 translation = -(vmin + vmax) * 0.5f + center; //원점으로 옮긴 후 + center
	 
	for (int i = 0; i < result.meshNum; i++) {
		for (int j = 0; j < result.meshes[i].verticesNum; j++) {
			result.meshes[i].vertices[j].Pos = (result.meshes[i].vertices[j].Pos + translation) * scale;
			//result.meshes[i].vertices[j].Normal = (result.meshes[i].vertices[j].Normal + translation) * scale;

		}
	}
	
	modelLoader.defaultTransform =  Matrix::CreateTranslation(translation)* Matrix::CreateScale(scale);
	result.m_defaultTransform = modelLoader.defaultTransform;
}