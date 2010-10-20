// Commonly used data structures and utility functions
// Placed here to avoid circular header dependencies
#ifndef COMMON_H_
#define COMMON_H_

#include <vector>
#include <string>
#include <sstream>
#include "Types.h"
#include "Poly.h"


//#define DEBUGGING


struct Bounds
{
	int x, y;
	int width, height, depth;
};

struct Stroke
{
	int length;
	Bounds bounds;
	std::vector<int2> points;

	void CalculateBounds();
};

struct LineOfSymmetry
{
	int2 pointOnLine;
	// Unit-vector for direction
	float2 direction;

	// Unit vectors for perpendicular vectors (direction rotated CCW and CW by 0.5pi)
	float2 ccwPerp, cwPerp;

	LineOfSymmetry();
	
	// Normalises direction and calculates ccwPerp and cwPerp
	void CalculateVectors();

	// Project a vector along the line given by ccwPerp/cwPerp, and return the magnitude of the resulting vector
	float ProjectedVectorMagnitude(float2 vec);

	// For debugging
	std::string ToString();

	int score;
};

struct Building
{
	Bounds bounds;
	std::vector<Poly> polys;
};


// Utility functions
float randFloat();

template<typename T>
std::string ConvertToString(T& arg)
{
	std::stringstream ss;
	ss << arg;
	return ss.str();
}








#endif