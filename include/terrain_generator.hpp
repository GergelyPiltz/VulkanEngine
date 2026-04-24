#pragma once

// std
#include <vector>

class TerrainGenerator {
public:
	TerrainGenerator(double*** scalarField, int cubesPerAxis);
	~TerrainGenerator() {}
	void createMeshData();
	std::vector<glm::vec3> getMeshData() { return vertices; }

private:
	double sample(glm::ivec3 pos);
	int getCubeCongif(double* cube);
	int CalculateVertex(glm::ivec3 position, int edgeIndex, double* cube);

	std::vector<glm::vec3> vertices{};
	std::vector<int> triangles{};
	int cubesPerAxis = 0;
	double*** scalarField;
	double terrainHeight = 0.0f;

};