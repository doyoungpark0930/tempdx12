#pragma once

class ModelLoader;
class GeometryGenerator
{
public:
	static MeshDataInfo testReadFromFile(char* basePath, const char* filename);
	static MeshDataInfo ReadFromFile(char* basePath, const char* filename);
	static MeshDataInfo AnimationReadFromFile(char* basePath, const char* filename, const char** animationNames, UINT animationNum);
	static void Normalize(const Vector3 center,
		const float longestLength, MeshDataInfo& result, ModelLoader& modelLoader);

};