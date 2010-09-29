#include "Clip.h"

#include <algorithm>
#include <set>


std::vector< std::vector<float2> > Clip(const std::vector<int2>& input, int yMin, int yMax)
{
	assert(input.size() != 0);

	std::vector<float2> clipped;
	std::vector<unsigned> outTop; // Indices of vertices created as an edge left the allowed range
	std::vector<unsigned> inTop; // Indices of vertices created as an edge entered the allowed range
	std::vector<unsigned> outBottom; // Indices of vertices created as an edge left the allowed range
	std::vector<unsigned> inBottom;

	int2 previous = input.back(); // First point has last point as previous
	for (unsigned i = 0; i < input.size(); i++)
	{
		int2 current = input[i];
		// Detect crossing over of range boundaries and insert extra vertices
		// NOTE: It turns out order of these is important. Consider the case where a single edge
		// comes in the bottom and out the top. We do not want to generate the top vertex first.
		if (previous.y <= yMin && current.y > yMin)
		{
			inBottom.push_back(clipped.size()); // Record current index as entry to range
			clipped.push_back(cutLine(previous, current, yMin)); // Add vertex at intersection
		}

		if (previous.y < yMax && current.y >= yMax)
		{
			outTop.push_back(clipped.size());
			clipped.push_back(cutLine(previous, current, yMax));
		}

		if (previous.y >= yMax && current.y < yMax)
		{
			inTop.push_back(clipped.size());
			clipped.push_back(cutLine(previous, current, yMax));
		}

		if (previous.y > yMin && current.y <= yMin)
		{
			outBottom.push_back(clipped.size()); // Record current index as exit
			clipped.push_back(cutLine(previous, current, yMin));
		}

		// Record the vertex itself if it is within the range
		if (current.y > yMin && current.y < yMax)
			clipped.push_back(float2(float(current.x), float(current.y)));

		previous = current;
	}

	// Sort in and out, so that first in is at same index as first out
	std::sort(outTop.begin(), outTop.end(), PositionalSort(clipped));
	std::sort(inTop.begin(), inTop.end(), PositionalSort(clipped));
	std::sort(outBottom.begin(), outBottom.end(), PositionalSort(clipped));
	std::sort(inBottom.begin(), inBottom.end(), PositionalSort(clipped));

	std::vector< std::vector<float2> > polys;
	std::set<unsigned> unprocessed; // Set indices of unprocessed vertices.
	for (unsigned i = 0; i < clipped.size(); i++)
		unprocessed.insert(i);

	while (!unprocessed.empty())
	{
		std::vector<float2> poly;
		unsigned v = *unprocessed.begin(); // Get index of first unprocessed vertex
		while (unprocessed.find(v) != unprocessed.end())
		{
			poly.push_back(clipped[v]);
			unprocessed.erase(v);

			std::vector<unsigned>::iterator outTopIndex = std::find(outTop.begin(), outTop.end(), v);
			std::vector<unsigned>::iterator outBottomIndex = std::find(outBottom.begin(), outBottom.end(), v);
			
			if (outTopIndex != outTop.end()) // If this index leaves out the top
				v = inTop[outTopIndex - outTop.begin()]; // Find where it returns
			else if (outBottomIndex != outBottom.end())
				v = inBottom[outBottomIndex - outBottom.begin()];
			else
				v = (v + 1) % clipped.size();
		}

		polys.push_back(poly);
	}

	return polys;
}



float2 cutLine(int2 a, int2 b, int height)
{
	float x = a.x + float(height - a.y) / (b.y - a.y) * (b.x - a.x);
	return float2(x, float(height));
}