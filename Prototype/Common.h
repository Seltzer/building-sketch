// Commonly used data structures and utility functions
// Placed here to avoid circular header dependencies
#ifndef COMMON_H_
#define COMMON_H_

#include <vector>
#include <string>
#include <sstream>
#include "Types.h"
#include "Poly.h"


#define DEBUGGING


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
	// Unit-vectors for direction and perpendicular direction (direction rotated CCW by 0.5pi)
	float2 direction, perpDirection;

	LineOfSymmetry();
	// For debugging
	std::string ToString();

	unsigned score;
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