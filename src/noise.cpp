#include "noise.hpp"
#include "PerlinNoise.hpp"

Noise::Noise(int seed) : seed( seed ) {
	perlin = siv::PerlinNoise(seed);
};

double** Noise::Generate(int width, int height) {
	int32_t octaves = 1;
	double frequency = 1.0f; 

	double** noise = new double*[height];
	
	const double fx = (frequency / width);
	const double fy = (frequency / height);

	for (std::int32_t y = 0; y < height; ++y)
	{
		noise[y] = new double[width];;
		for (std::int32_t x = 0; x < width; ++x)
		{
			noise[y][x] = perlin.octave2D_01((x * fx), (y * fy), octaves);
		}
	}

	return noise;
}
