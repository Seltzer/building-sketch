#include <iostream>
#include "SketchPreprocessing.h"

using namespace std;

vector<int2> DouglasPeuker(vector<int2>::const_iterator start, vector<int2>::const_iterator end, float threshold)
{
	vector<int2> result;
	if (end - start <= 1) { // Can't subdivide any more
		result.push_back(*start);
		return result;
	}

	// Find line parameters
	int2 lineStart = *start;
	int2 lineEnd = *end;
	int2 diff = lineEnd - lineStart;
	float2 dir = normal(float2(float(diff.x), float(diff.y)));
	float2 ortho(-dir.y, dir.x);

	// Find point with greatest error
	vector<int2>::const_iterator mid = start + 1;
	float maxError = 0;
	for (vector<int2>::const_iterator i = start + 1; i != end; i++)
	{
		int2 rel = *i - lineStart;
		float dist = dot(ortho, float2(float(rel.x), float(rel.y)));
		float error = dist * dist; // Squared distance
		if (error > maxError)
		{
			maxError = error;
			mid = i;
		}
	}

	if (maxError >= threshold)
	{
		result = DouglasPeuker(start, mid, threshold);
		vector<int2> rhs = DouglasPeuker(mid, end, threshold);
		result.insert(result.end(), rhs.begin(), rhs.end());
		return result;
	}

	// Insufficient error, just return single line segment
	result.push_back(*start);
	return result;
}

Stroke Reduce(const Stroke& stroke, float threshold)
{
	if (stroke.points.size() <= 2)
		return stroke; // Cannot reduce any further
	vector<int2> result = DouglasPeuker(stroke.points.begin(), stroke.points.end() - 1, threshold);
	result.push_back(stroke.points.back()); // The recursion will never insert the last point.
	Stroke final;
	final.length = stroke.points.size();
	final.points = result;
	return final;
}

unsigned NumberOfPoints(std::vector<Stroke>& sketch)
{
	unsigned num = 0;

	for (vector<Stroke>::iterator it = sketch.begin(); it < sketch.end(); it++)
		num += (*it).points.size();

	return num;

}