#pragma once

#include "PerlinNoise.hpp"

class Noise {
public:
	Noise(int seed);
	double** Generate(int width, int height);

private:
	int seed;
	siv::PerlinNoise perlin;
};