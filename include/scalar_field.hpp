#pragma once

class ScalarField {
public:
	ScalarField(double** heightMap, int width, int height, int depth);
	double*** get() { return scalarField; }
private:
	double*** scalarField;
};