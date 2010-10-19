#include "SymmetryDetection.h"

#include <iostream>
#include <map>

using namespace std;



bool PointExistsNear(bool pixels[805][605], const int2 position)
{
	// TODO this can all be precalculated
	return pixels[position.x][position.y] || pixels[position.x - 1][position.y - 1] || pixels[position.x - 1][position.y] || pixels[position.x - 1][position.y + 1] ||
											 pixels[position.x + 1][position.y - 1] || pixels[position.x + 1][position.y] || pixels[position.x + 1][position.y + 1] ||
										  pixels[position.x][position.y - 1] || pixels[position.x][position.y + 1];
}


bool EdgeExistsBetween(bool pixels[805][605], const int2 pos1, const int2 pos2)
{
	int2 edgeVector = pos2 - pos1;
	float length = sqrt((float) dot(edgeVector, edgeVector));
	if (length <= 1)
		return true;

	float interval = 1.00f / ceil(length);


	for (float t = 0; t < 1; t += interval)
	{
		int2 pointToInspect = pos1 + edgeVector * t;

		if (!PointExistsNear(pixels, pointToInspect))
			return false;
	}

	return true;
}



bool operator <(const int2& a, const int2& b)
{
	if (a[0] != b[0])
		return a[0] < b[0];
	else
		return a[1] < b[1];
}



void EvaluateLOS(LineOfSymmetry& los, bool pixels[805][605], vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	unsigned matches = 0;

	// Mapping midpoints to number of matches (this is used to mitigate effects of edges which have many matches
	map<int2,int> midpointCount;

	// Mapping distances to number of matches (used to measure spread of matches around LOS)
	map<int,int> matchDistribution;
	

	// Max distance a match point is from LOS
	unsigned maxDistance = 0;

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
			int dist = pointToLosDistance;
			
			// Vector between point and LOS
			float2 vectorToLOS = pointToLosDistance * los.perpDirection;

			// Calculate midpoint and counterpart
			float2 midpointPosition = p + vectorToLOS;
			float2 counterpartPosition = midpointPosition + vectorToLOS;
			int2 counterpartPosAsInts(counterpartPosition.x, counterpartPosition.y);

			// This is true when p lies on the line of symmetry - this will result in a match which we should ignore
			if (p == counterpartPosAsInts)
				continue;

			// Give more weighting for more points existing near the counterpart position???


			if ( (counterpartPosAsInts.x > 0) && (counterpartPosAsInts.x < 800) )
			{
				if ( (counterpartPosAsInts.y > 0) && (counterpartPosAsInts.y < 600) )
				{
					if (PointExistsNear(pixels, counterpartPosAsInts))
					{
						// TODO move this for optimisation
						if (dist < 0)
							continue;


						int2 midpointAsInts(midpointPosition.x, midpointPosition.y);
						int matchesAtMidpoint = midpointCount.count(midpointAsInts);

						// If there are already >4 matches centered on this midpoint, then there's a good chance we're dealing
						// with an edge... try to determine this
						if (matchesAtMidpoint > 4)
						{
							if (EdgeExistsBetween(pixels, p, counterpartPosAsInts))
								continue;
						}
						else if (matchesAtMidpoint == 0)
						{
							midpointCount[midpointAsInts] = 0;
						}
						else
						{
							midpointCount[midpointAsInts] = matchesAtMidpoint + 1;
						}

						// Update matchDistribution
						if (matchDistribution.count(dist))
							matchDistribution[dist] = matchDistribution[dist] + 1;
						else
							matchDistribution[dist] = 1;

						if (dist > maxDistance)
							maxDistance = dist;

						matches++;
					}
				}
			}
			
			// for testing
			if (pointToLosDistance < 0)
				mirroredStroke1.points.push_back(int2(counterpartPosition.x, counterpartPosition.y));
			else if (pointToLosDistance > 0)
				mirroredStroke2.points.push_back(int2(counterpartPosition.x, counterpartPosition.y));
		}
	}

	// Calculate score
	float coefficient = 1;
		
	if (matches > 0)
	{
		// Max spread assumes a symmetric sketch with all points distributed to the very left and very right
		float maxSpread = (buildingOutline.bounds.width / 2.00f) * (matches / 2.00f);
		float spread = 0;

		for (map<int,int>::iterator it = matchDistribution.begin(); it != matchDistribution.end(); it++)
			spread += (*it).first * (*it).second;

		coefficient = ((maxSpread + spread) / maxSpread);

		#ifdef DEBUGGING
		if (!strokes.empty())
		{
			cout << "spread = " << spread << " and maxSpread = " << maxSpread << endl;
			cout << "coefficient = " << coefficient << endl;
		}
		#endif
	}
	
	los.score =  matches * coefficient;

	#ifdef DEBUGGING
		if (!strokes.empty() && matches > 0)
		{
			cout << "# stroke points = " << strokes[0].points.size() << endl;
			cout << "# matches = " << matches << endl;
		}
	#endif


}



LineOfSymmetry CalculateSymmetry(bool pixels[805][605], vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	LineOfSymmetry los;
	buildingOutline.CalculateBounds();
		
	// For now, assume a vertical gradient for the los
	los.direction = float2(0,1);
	//los.direction = float2(0.4,1);
	los.direction = normal(los.direction);
	los.perpDirection = float2(-los.direction.y, los.direction.x);
	
	
	unsigned bestMatch = 0;
	unsigned bestX = 0;		// hack
	float bestDeltaX = 0;
	
	
	for (float deltaX = 0; deltaX <= 0; deltaX += 0.05)
	{
		los.direction = float2(deltaX, 1);
		los.direction = normal(los.direction);

		for (int x = 0; x < buildingOutline.bounds.width; x++)
		{
			los.pointOnLine = int2(buildingOutline.bounds.x + x, buildingOutline.bounds.y);
			
			EvaluateLOS(los, pixels, strokes, buildingOutline, mirroredStroke1, mirroredStroke2);
		
			if (los.score > bestMatch)
			{
				bestMatch = los.score;
				bestX = x;
				bestDeltaX = deltaX;
			}
		}
	}


	los.pointOnLine = int2(buildingOutline.bounds.x + bestX, buildingOutline.bounds.y);
	los.direction = float2(bestDeltaX, 1);
	los.direction = normal(los.direction);

	mirroredStroke1.points.clear();
	mirroredStroke2.points.clear();
		
	EvaluateLOS(los, pixels, strokes, buildingOutline, mirroredStroke1, mirroredStroke2);

	if (los.score == 0)
		cout << "no symmetry detected (not enough points???)" << endl;

	return los;
}