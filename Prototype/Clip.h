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
/*std::vector<float2> Clip(const std::vector<int2>& input, int yMin, int yMax)
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
}*/

// Predicate for sorting inices based on vertex x coordinate
struct PositionalSort
{
	std::vector<float2> verts;
	PositionalSort(const std::vector<float2>& verts) : verts(verts) {}

	bool operator()(int a, int b)
	{
		float2 pa = verts[a];
		float2 pb = verts[b];
		if (pa.x == pb.x)
			return pa.y < pb.y;
		return pa.x < pb.x;
	}
};

// Improved clipping algorithm which can produce multiple polygons, based on
// Graphics Gems V, chapter 11.3, clipping a concave polygon
// http://books.google.co.nz/books?id=S4n3qj_5C0gC&pg=PA50&lpg=PA50&dq=concave+polygon+clipping&source=bl&ots=u5mZxyyE7N&sig=NryvE7GCsxro2GlTFbJKnKr8UDc&hl=en&ei=3LFoTIuwG5P4swP7gp3jDQ&sa=X&oi=book_result&ct=result&resnum=6&ved=0CDQQ6AEwBQ#v=onepage&q=concave%20polygon%20clipping&f=false
// This implementation is slightly improved, in that it can survive some self intersecting cases (not all)
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
		if (previous.y < yMin && current.y >= yMin)
		{
			inBottom.push_back(clipped.size()); // Record current index as entry to range
			clipped.push_back(cutLine(previous, current, yMin)); // Add vertex at intersection
		}
		if (previous.y >= yMin && current.y < yMin)
		{
			outBottom.push_back(clipped.size()); // Record current index as exit
			clipped.push_back(cutLine(previous, current, yMin));
		}

		if (previous.y > yMax && current.y <= yMax)
		{
			inTop.push_back(clipped.size());
			clipped.push_back(cutLine(previous, current, yMax));
		}
			
		if (previous.y <= yMax && current.y > yMax)
		{
			outTop.push_back(clipped.size());
			clipped.push_back(cutLine(previous, current, yMax));
		}

		// Record the vertex itself if it is within the range
		if (current.y >= yMin && current.y <= yMax)
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

#endif //CLIP_H
