#include <iostream>
#include "SketchPreprocessing.h"
#include "Common.h"

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


/* Currently only checks if a point is AT position - it used to check points in the vicinity of position
 * but that has temporarily been removed due to speed issues.
 *
 * TODO: This is a prime candidate for algorithm optimisation - strokes and points should be partitioned 
 *       by area so that the neighbourhood of position can quickly be discovered
 */
bool PointExistsNear(vector<Stroke>& strokes, const int2 position)
{
	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			int2& point = (*it2);

			if (point == position)
				return true;
		}
	}

	return false;
}




// Evaluate a line of symmetry, and return a heuristic score
unsigned EvaluateLOS(const LineOfSymmetry& los, vector<Stroke>& strokes, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	unsigned matches = 0;

	// Calculate distance from origin to line of symmetry
	// This is the projection of the los position vector on the los perpendicular vector
	float originToLosDistance = fabs((float) dot(los.pointOnLine,los.perpDirection));

	#ifdef DEBUGGING
		cout << "Distance of los from origin is " << originToLosDistance << endl;
	#endif


	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			// For current point p:
			float2 p = *it2;
		
			// Calculate magnitude of projection of p (the point's position vector) on LOS
			int proj = fabs((float) dot(p, los.perpDirection));

			float pointToLosDistance = proj - originToLosDistance;
			

			float2 counterpartPosition = p + los.perpDirection * pointToLosDistance * 2;
			int2 counterpartPosAsInts = int2(counterpartPosition.x, counterpartPosition.y);

			// This is true when p lies on the line of symmetry
			if (p == counterpartPosAsInts)
				continue;

			// Give more weighting for more points existing near the counterpart position???
			if (PointExistsNear(strokes, counterpartPosAsInts))
				matches++;

			// for testing
			if (pointToLosDistance < 0)
				mirroredStroke1.points.push_back(int2(counterpartPosition.x, counterpartPosition.y));
			else if (pointToLosDistance > 0)
				mirroredStroke2.points.push_back(int2(counterpartPosition.x, counterpartPosition.y));
		}
	}

	#ifdef DEBUGGING
		if (!strokes.empty())
			cout << "# stroke points = " << strokes[0].points.size() << endl;
		cout << "# matches = " << matches << endl;
	#endif


	return matches;
}



LineOfSymmetry CalculateSymmetry(vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	LineOfSymmetry los;
		
	// For now, assume a vertical gradient for the los
	los.direction = float2(0,1);
	//los.direction = float2(0.4,1);
	los.direction = normal(los.direction);
	los.perpDirection = float2(-los.direction.y, los.direction.x);
	
	
	unsigned bestMatch = 0;
	unsigned bestX = 0;		// hack
	
	
	for (int x = 0; x < buildingOutline.bounds.width; x++)
	{
		los.pointOnLine = int2(buildingOutline.bounds.x + x, buildingOutline.bounds.y);
			
		unsigned matches = EvaluateLOS(los, strokes, mirroredStroke1, mirroredStroke2);

		if (matches > bestMatch)
		{
			bestMatch = matches;
			bestX = x;
		}
	}


	los.pointOnLine = int2(buildingOutline.bounds.x + bestX, buildingOutline.bounds.y);

	mirroredStroke1.points.clear();
	mirroredStroke2.points.clear();
		
	EvaluateLOS(los, strokes, mirroredStroke1, mirroredStroke2);

	return los;
}