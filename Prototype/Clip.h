#ifndef CLIP_H
#define CLIP_H

#include "Types.h"

#include <vector>

// Find intersection point of line segment with horizontal line at given height
float2 cutLine(int2 a, int2 b, int height)
{
	float x = a.x + float(height - a.y) / (b.y - a.y) * (b.x - a.x);
	return float2(x, float(height));
}

// Sutherland-Hodgeman algorithm based on description at http://www.codeguru.com/cpp/misc/misc/graphics/article.php/c8965
std::vector<float2> Clip(const std::vector<int2>& input, int yMin, int yMax)
{
	assert(input.size() != 0);

	std::vector<float2> clipped;

	int2 previous = input.back(); // First point has last point as previous
	for (unsigned i = 0; i < input.size(); i++)
	{
		int2 current = input[i];
		// If intersecting yMin
		if ((previous.y < yMin && current.y >= yMin) || (previous.y >= yMin && current.y < yMin))
			clipped.push_back(cutLine(previous, current, yMin));

		if ((previous.y > yMax && current.y <= yMax) || (previous.y <= yMax && current.y > yMax))
			clipped.push_back(cutLine(previous, current, yMax));

		if (current.y >= yMin && current.y <= yMax)
			clipped.push_back(float2(float(current.x), float(current.y)));

		previous = current;
	}

	return clipped;
}

#endif //CLIP_H
