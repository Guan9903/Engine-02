#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap(const std::string& name)
{
	int iWidth, iHeight, iChans;	//iChans: Channel count
	GLubyte* data = SOIL_load_image(name.c_str(), &iWidth, &iHeight, &iChans, 1);

	if (!data)
	{
		std::cout << "Heightmap can't load file !\n";
		return;
	}

	numVertices		= iWidth * iHeight;
	Vector3 vertexScale		= Vector3(4.0f, 1.0f, 4.0f);
	Vector2 textureScale	= Vector2(1/1024.0f, 1/1024.0f);
	int i = 0;

	for (int z = 0; z < iHeight - 1; z++)
	{
		for (int x = 0; x < iWidth - 1; x++)
		{
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = (z + 1) * (iWidth) + (x + 1);
			int d = (z + 1) * (iWidth) + x;

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

	heightMapSize.x = vertexScale.x * (iWidth - 1);
	heightMapSize.y = vertexScale.y * 255.0f;	//Each height is a byte
	heightMapSize.z = vertexScale.z * (iHeight - 1);
}