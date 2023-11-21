#pragma once
#include <string>
#include "Mesh.h"

class HeightMap : public Mesh
{
public:
	HeightMap(const std::string& name, float xScale, float yScale, float zScale);
	~HeightMap(void) {};

	Vector3 GetHeightmapSize() const { return heightmapSize; }
	float GetHeightAtXZ(float x, float z);
protected:
	Vector3 heightmapSize;
	float xScale;
	float yScale;
	float zScale;
};