// Commonly used data structures and utility functions
// Placed here to avoid circular header dependencies
#ifndef COMMON_H_
#define COMMON_H_

#include <vector>
#include <string>
#include <sstream>
#include "Types.h"
#include "Poly.h"

struct Bounds
{
	int x;
	int y;
	int width;
	int height;
	int depth;
};

struct Stroke
{
	int length;
	Bounds bounds;
	std::vector<int2> points;

	void CalculateBounds();
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