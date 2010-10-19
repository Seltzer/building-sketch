#include "Common.h"
#include <sstream>

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
	: pointOnLine(0,0), direction(0,0), perpDirection(0,0), score(0)
{
}


string LineOfSymmetry::ToString()
{
	stringstream ss;
	ss << "********** pointOnLine = " << pointOnLine.tostring() << "\n";
	ss << "********** direction = " << direction.tostring() << "\n";
	ss << "********** perpDirection = " << perpDirection.tostring() << "\n";

	return ss.str();
}