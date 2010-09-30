// Commonly used data structures and utility functions
// Placed here to avoid circular header dependencies
#ifndef COMMON_H_
#define COMMON_H_

#include <vector>
#include "Types.h"



typedef std::vector<float3> Poly;


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
};

struct Building
{
	Bounds bounds;
	std::vector<Poly> polys;
};


// Utility functions
float randFloat();








#endif