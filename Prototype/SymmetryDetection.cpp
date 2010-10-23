#include "SymmetryDetection.h"

#include <iostream>
#include <map>
#include <stack>
#include <deque>


using namespace std;



bool PointExistsNear(bool pixels[805][605], bool pointNear[805][605], const int2 position)
{
	return pixels[position.x][position.y];
//	return pointNear[position.x][position.y];
}


bool EdgeExistsBetween(bool pixels[805][605], bool pointNear[805][605],const int2 pos1, const int2 pos2)
{
	int2 edgeVector = pos2 - pos1;
	float length = sqrt((float) dot(edgeVector, edgeVector));
	if (length <= 1)
		return true;

	float interval = 1.00f / ceil(length);


	for (float t = 0; t < 1; t += interval)
	{
		int2 pointToInspect = pos1 + edgeVector * t;

		if (!PointExistsNear(pixels, pointNear, pointToInspect))
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



void EvaluateLOS(LineOfSymmetry& los, bool pixels[805][605], bool pointNear[805][605], vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	// Number of pair matches found so far
	unsigned matches = 0;
	// Mapping midpoints to number of matches (this is used to mitigate effects of edges which have many matches
	map<int2,int> midpointCount;
	// Mapping distances to number of matches (used to measure spread of matches around LOS)
	map<int,int> matchDistribution;
	// Max distance a match point is from LOS
	unsigned maxDistance = 0;



	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			// For current point p:
			float2 p = *it2;
					
			// Calculate magnitude of projection of p (the point's position vector) on LOS perpendicular
			float proj = los.ProjectedVectorMagnitude(p);

			// Negative if the LOS is in between the origin and p. Positive otherwise
			float pointToLosDistance = proj - los.distanceFromOrigin;
			

			// Calculate vector between point and LOS
			float2 vectorToLOS = pointToLosDistance * los.ccwPerp;

			// Calculate midpoint and counterpart
			float2 midpointPosition = p + vectorToLOS;
			//cout << "\t midpointPosition = " << midpointPosition.tostring() << " / " << originToLosDistance << endl;

			float2 counterpartPosition = midpointPosition + vectorToLOS;
			int2 counterpartPosAsInts(counterpartPosition.x, counterpartPosition.y);

			// Draw counterpart
			mirroredStroke1.points.push_back(int2(counterpartPosition.x, counterpartPosition.y));



			if ( (counterpartPosAsInts.x >= 0) && (counterpartPosAsInts.x <= 800) )
			{
				if ( (counterpartPosAsInts.y >= 0) && (counterpartPosAsInts.y <= 600) )
				{
					// This is true when p lies on the line of symmetry - this will result in a match which we should ignore
					if (p == counterpartPosAsInts)
						continue;

					if (PointExistsNear(pixels, pointNear, counterpartPosAsInts))
					{
						int dist = pointToLosDistance;

						int2 midpointAsInts(midpointPosition.x, midpointPosition.y);
						int matchesAtMidpoint = midpointCount.count(midpointAsInts);

						// If there are already >4 matches centered on this midpoint, then there's a good chance we're dealing
						// with an edge... try to determine this
						if (matchesAtMidpoint > 4)
						{
							if (EdgeExistsBetween(pixels, pointNear, p, counterpartPosAsInts))
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
						int absDist = (int) fabs(pointToLosDistance);

						if (matchDistribution.count(absDist))
							matchDistribution[absDist] = matchDistribution[absDist] + 1;
						else
							matchDistribution[absDist] = 1;

						if (absDist > maxDistance)
							maxDistance = dist;

						matches++;
					}
				}
			}
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



LineOfSymmetry CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605], vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2)
{
	LineOfSymmetry los;
	los.CalculateVectors();

	buildingOutline.CalculateBounds();
		
	
	int bestMatch = -100000;
	// hack
	unsigned bestX = 0;	
	float bestDeltaX = 0;
	
	
	for (float deltaX = 0; deltaX <= 0.4; deltaX += 0.05)
	//for (float deltaX = 0; deltaX <= 0; deltaX += 0.05)
	{
		cout << "deltaX = " << deltaX << endl;

		los.direction = float2(deltaX,1);
		
		for (int x = 0; x < buildingOutline.bounds.width; x++)
		{
			los.pointOnLine = int2(buildingOutline.bounds.x + x, buildingOutline.bounds.y);
			los.CalculateVectors();
		//	cout << "trying at " << buildingOutline.bounds.x + x << endl;
			
			EvaluateLOS(los, pixels, pointNear, strokes, buildingOutline, mirroredStroke1, mirroredStroke2);
		
			//cout << "los.score = " << los.score << endl;
			//cout << "bestMatch = " << bestMatch << endl;

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
	los.CalculateVectors();

	mirroredStroke1.points.clear();
	mirroredStroke2.points.clear();
		
	EvaluateLOS(los, pixels, pointNear, strokes, buildingOutline, mirroredStroke1, mirroredStroke2);

	if (los.score == 0)
		cout << "no symmetry detected (not enough points???)" << endl;
	else
		cout << "Score = " << los.score << endl;

	if (strokes.size() > 0)
		cout << strokes[0].points.size() << " points " << endl;


	return los;
}


vector<Stroke> ApplyLOS(LineOfSymmetry& los, bool pixels[805][605], bool pointNear[805][605], vector<Stroke>& strokes, Stroke& buildingOutline, 
						Stroke& mirroredStroke1, Stroke& mirroredStroke2, bool mirrorLeft)
{
	vector<Stroke> outputStrokes;


	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		vector<Stroke> generatedStrokes;
		generatedStrokes.push_back(Stroke());
		int currentStroke = 0;

		stack<int2> pointsStack;
		deque<int2> pointsQueue;


		// Are we on the LOS or to the left or right?
		enum Orientation { LEFT, CENTRE, RIGHT };
		Orientation or = LEFT;

		bool greedyMode = true;


		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			// For current point p:
			float2 p = *it2;
					
			// Calculate magnitude of projection of p (the point's position vector) on LOS
			float proj = los.ProjectedVectorMagnitude(p);
			// Negative if the LOS is in between the origin and p. Positive otherwise
			float pointToLosDistance = proj - los.distanceFromOrigin;
				

			if (mirrorLeft)
			{
				if (pointToLosDistance > 0)
				{
					// Point is to the right of LOS

					if ( (or == LEFT) || (or == CENTRE) )
					{
						// Transition from left/centre to right
						
						// Add points on stack to new stroke 
						// Could be primary or counterparts, depending on whether we were in greedy mode - we don't care
						generatedStrokes.push_back(Stroke());
						++currentStroke;

						while(!pointsStack.empty())
						{
							generatedStrokes[currentStroke].points.push_back(pointsStack.top());
							pointsStack.pop();
						}


						if (!greedyMode)
						{
							// Add points in queue to new stroke - these are primaries
							generatedStrokes.push_back(Stroke());
							++currentStroke;

							while(!pointsQueue.empty())
							{
								generatedStrokes[currentStroke].points.push_back(pointsQueue.front());
								pointsQueue.pop_front();
							}
						}
												
						greedyMode = false;
					}

					or = RIGHT;
				}
				else if (pointToLosDistance == 0)
				{
					// Point is on LOS
					or = CENTRE;

					if (greedyMode)
						generatedStrokes[currentStroke].points.push_back(p);
					else
						pointsQueue.push_back(p);
				}
				else
				{
					// Point is to left of LOS
					or = LEFT;
					
					// Calculate position of counterpart point
					float2 counterpartPosition = p + 2 * pointToLosDistance * los.ccwPerp; 
					int2 counterpartPosAsInts(counterpartPosition.x, counterpartPosition.y);


					if (greedyMode)
					{
						generatedStrokes[currentStroke].points.push_back(p);
						pointsStack.push(counterpartPosAsInts);
					}
					else
					{
						pointsStack.push(counterpartPosAsInts);
						pointsQueue.push_back(p);
					}
				}
			}
			else
			{
				assert(false);
			}
		}


		// We've run out of points - flush the queue + stack
		generatedStrokes.push_back(Stroke());
		++currentStroke;

		while(!pointsStack.empty())
		{
			generatedStrokes[currentStroke].points.push_back(pointsStack.top());
			pointsStack.pop();
		}
		
		if (!greedyMode)
		{
			// Add points in queue to new stroke - these are primaries
			generatedStrokes.push_back(Stroke());
			++currentStroke;

			while(!pointsQueue.empty())
			{
				generatedStrokes[currentStroke].points.push_back(pointsQueue.front());
				pointsQueue.pop_front();
			}
		}

		for (vector<Stroke>::iterator it = generatedStrokes.begin(); it < generatedStrokes.end(); it++)
		{
			outputStrokes.push_back(*it);

		}
	}

	// Remove points outside bounds
	// Combine strokes if possible

	// return new strokes
	return outputStrokes;
}
