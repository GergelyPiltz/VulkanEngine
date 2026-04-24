#include "scalar_field.hpp"

// std
#include <iostream>

ScalarField::ScalarField(double** heightMap, int X, int Y, int Z) {
	scalarField = new double**[X];

	for (int _x = 0; _x < X; _x++) {
		scalarField[_x] = new double*[Y];
		for (int _y = 0; _y < Y; _y++) {
			scalarField[_x][_y] = new double[Z];
		}
	}

	//for (int _x = 0; _x < X; _x++) {
	//	for (int _y = 0; _y < Y; _y++) {
	//		for (int _z = 0; _z < Z; _z++)
	//		{
	//			scalarField[_x][_z][_y] = heightMap[_x][_y] - _z / (float)Z; //Z * heightMap[_x][_y] - (double)_z;
	//			//((i / (float)height) - noise);
	//		}
	//	}
	//}

	for (int _x = 0; _x < X; _x++) {
		for (int _y = 0; _y < Y; _y++) {
			for (int _z = 0; _z < Z; _z++)
			{
				// Both result in correct orientation
				scalarField[_x][_y][_z] = heightMap[_x][_z] - _y / (float)Z;
				//scalarField[_x][_z][_y] = heightMap[_x][_y] - _z / (float)Z;
			}
		}
	}
	return;
	for (int _y = 0; _y < Y; _y++) {
		for (int _x = 0; _x < X; _x++) {
			for (int _z = 0; _z < Z; _z++) {
				if (scalarField[_x][_y][_z] > 0.0f) std::cout << "#";
				if (scalarField[_x][_y][_z] <= 0.0f) std::cout << " ";				
				//std::cout << std::round(scalarField[_x][_y][_z] * 100) / 100 << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}