#include "tables.hpp"
#include "terrain_generator.hpp"

TerrainGenerator::TerrainGenerator(double*** scalarField, int cubesPerAxis) : scalarField( scalarField ), cubesPerAxis( cubesPerAxis ){}

double TerrainGenerator::sample(glm::ivec3 pos) {
    return scalarField[pos.x][pos.y][pos.z];
}

// <summary>
// Calculates the configuration of a single cube
// </summary>
// <param name="cube">Float[] of the corners of the cube.</param>
int TerrainGenerator::getCubeCongif(double* cube) {
    int configIndex = 0;
    for (int i = 0; i < 8; i++)
        if (cube[i] > terrainHeight)
            configIndex |= 1 << i;
    return configIndex;
}

void TerrainGenerator::createMeshData()
{
    vertices.clear();
    triangles.clear();

    for (int x = 0; x < cubesPerAxis; x++)
        for (int y = 0; y < cubesPerAxis; y++)
            for (int z = 0; z < cubesPerAxis; z++)
            {
                glm::ivec3 position{ x, y, z };

                double cube[8];
                for (int i = 0; i < 8; i++)
                    cube[i] = sample(position + Tables::corners[i]);

                int configIndex = getCubeCongif(cube);

                if (configIndex == 0 || configIndex == 255) continue;

                for (int vertexCounter = 0; vertexCounter < 15; vertexCounter++)
                {
                    int edgeIndex = Tables::triangles[configIndex][vertexCounter];

                    if (edgeIndex == -1) break;

                    CalculateVertex(position, edgeIndex, cube);
                    
                }
            }
}

/// <summary>
/// Calculates a single vertex
/// </summary>
int TerrainGenerator::CalculateVertex(glm::ivec3 position, int edgeIndex, double* cube)
{
    glm::vec3 vert1 = Tables::corners[Tables::edges[edgeIndex][0]];
    glm::vec3 vert2 = Tables::corners[Tables::edges[edgeIndex][1]];

    glm::vec3 vertPos;
    if (false)
    {
        float vert1Sample = cube[Tables::edges[edgeIndex][0]];
        float vert2Sample = cube[Tables::edges[edgeIndex][1]];

        float difference = vert2Sample - vert1Sample;

        difference = (terrainHeight - vert1Sample) / difference;

        vertPos = vert1 + (vert2 - vert1) * difference;
    }
    else
        vertPos = (vert1 + vert2) / 2.0f;

    vertPos += position;

    vertices.push_back(vertPos);
    int vertexCount = vertices.size();
    triangles.push_back(vertexCount - 1);

    return (vertexCount - 1);
}

