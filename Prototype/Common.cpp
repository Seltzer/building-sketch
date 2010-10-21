#include "Common.h"
#include <sstream>
#include <iostream>

using namespace std;


float randFloat()
{
	return rand() / float(RAND_MAX);
}


void Stroke::CalculateBounds(void)
{
	if (points.size() == 0)
		return;

	int2 minCoords = int2(points[0].x,points[0].y);
	int2 maxCoords = int2(points[0].x,points[0].y);
	for (unsigned i = 0; i < points.size(); i++)
	{
		minCoords.x = (minCoords.x < points[i].x) ? minCoords.x : points[i].x;
		minCoords.y = (minCoords.y < points[i].y) ? minCoords.y : points[i].y;
		maxCoords.x = (maxCoords.x > points[i].x) ? maxCoords.x : points[i].x;
		maxCoords.y = (maxCoords.y > points[i].y) ? maxCoords.y : points[i].y;
	}

	// Store the bounds of the stroke
	bounds.x = minCoords.x;
	bounds.y = minCoords.y;
	bounds.width = abs(maxCoords.x - minCoords.x);
	bounds.height = abs(maxCoords.y - minCoords.y);
	bounds.depth = 0;
}



LineOfSymmetry::LineOfSymmetry()
	: pointOnLine(0,0), direction(0,0), ccwPerp(0,0), cwPerp(0,0), distanceFromOrigin(0), score(0)
{
}


float LineOfSymmetry::ProjectedVectorMagnitude(float2 vec)
{
	float dist = (float) dot(vec, cwPerp);
	
	if (dist < 0)
		dist = (float) dot(vec, ccwPerp);

//	assert(dist >= 0);
	return dist;
}

void LineOfSymmetry::CalculateVectors()
{
	direction = normal(direction);

	ccwPerp = float2(-direction.y, direction.x);
	cwPerp = float2(direction.y, -direction.x);

	distanceFromOrigin = ProjectedVectorMagnitude(pointOnLine);
}

string LineOfSymmetry::ToString()
{
	stringstream ss;
	ss << "********** pointOnLine = " << pointOnLine.tostring() << "\n";
	ss << "********** direction = " << direction.tostring() << "\n";
	ss << "********** ccwPerp = " << ccwPerp.tostring() << "\n";
	ss << "********** cwPerp = " << cwPerp.tostring() << "\n";

	return ss.str();
}