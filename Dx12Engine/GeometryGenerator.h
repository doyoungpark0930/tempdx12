#pragma once

class ModelLoader;
class GeometryGenerator
{
public:
	static MeshDataInfo ReadFromFile(char* basePath, const char* filename);
	static void Normalize(const Vector3 center,
		const float longestLength, MeshDataInfo& result, ModelLoader& modelLoader);

};