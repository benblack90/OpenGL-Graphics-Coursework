#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& name, float xScale, float yScale, float zScale)
{
	int iWidth, iHeight, iChans;															//iChans meaning 'channels'
	unsigned char* data = SOIL_load_image(name.c_str(), &iWidth, &iHeight, &iChans, 1);		//restrict output to one channel i.e.grayscale

	if (!data)
	{
		std::cout << "Heightmap can't load file!\n";
		return;
	}

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	this->xScale = xScale;
	this->yScale = yScale;
	this->zScale = zScale;
	Vector3 vertexScale = Vector3(xScale, yScale, zScale);
	Vector2 textureScale = Vector2(1 / zScale, 1 / zScale);

	for (int z = 0; z < iHeight; ++z)
	{
		for (int x = 0; x < iWidth; ++x)
		{
			if (x == 4900/16 && z == 7757/16)
			{
				int xTina = 12;
			}
			int offset = (z * iWidth) + x;
			vertices[offset] = Vector3(x, data[offset], z) * vertexScale;
			textureCoords[offset] = Vector2(x, z) * textureScale;
		}
	}
	SOIL_free_image_data(data);

	int i = 0; 

	for (int z = 0; z < (iHeight - 1); z++)
	{
		for (int x = 0; x < (iWidth - 1); x++)
		{
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = ((z + 1) * (iWidth)) + (x + 1);
			int d = ((z + 1) * (iWidth)) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}

	GenerateNormals();
	GenerateTangents();
	BufferData();

	heightmapSize.x = vertexScale.x * (iWidth - 1);
	heightmapSize.y = vertexScale.y * 255.0f;
	heightmapSize.z = vertexScale.z * (iHeight - 1);
}

float HeightMap::GetHeightAtXZ(float x, float z)
{
	int xIndex = x / xScale;
	int zIndex = z / zScale;
	int index = (heightmapSize.x/xScale + 1) * zIndex + xIndex;
	return vertices[index].y;
}


