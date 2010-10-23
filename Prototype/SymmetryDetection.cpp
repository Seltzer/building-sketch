#include "SymmetryDetection.h"

#include <iostream>
#include <map>


using namespace std;



// Utility functions
bool PointExistsNear(bool pixels[805][605], bool pointNear[805][605], const int2 position)
{
//	return pixels[position.x][position.y];
	return pointNear[position.x][position.y];
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


//////////////////////////////////////////////////////////// LineOfSymmetry Implementation
LineOfSymmetry::LineOfSymmetry()
	: pointOnLine(0,0), direction(0,0), ccwPerp(0,0), cwPerp(0,0), distanceFromOrigin(0), matches(0),score(0)
{
}

float LineOfSymmetry::ProjectedVectorMagnitude(float2 vec)
{
	float dist = (float) dot(vec, cwPerp);
	
	if (dist < 0)
		dist = (float) dot(vec, ccwPerp);

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
	ss << "********** cwPerp = " << matches << "\n";
	ss << "********** cwPerp = " << score;


	return ss.str();
}


//////////////////////////////////////////////////////////// SymmetryApplication Implementation
SymmetryApplication::SymmetryApplication(std::vector<Stroke>& strokes, Stroke& buildingOutline)
	: strokes(strokes), buildingOutline(buildingOutline)
{
}


void SymmetryApplication::CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605])
{
	los.CalculateVectors();

	buildingOutline.CalculateBounds();
		
	
	int bestMatch = -100000;
	// hack
	unsigned bestX = 0;	
	float bestDeltaX = 0;
	
	// TODO check that this tries 0
	for (float deltaX = -0.4; deltaX <= 0.4; deltaX += 0.05)
	{
		cout << "deltaX = " << deltaX << endl;

		los.direction = float2(deltaX,1);
		
		for (int x = 0; x < buildingOutline.bounds.width; x++)
		{
			los.pointOnLine = int2(buildingOutline.bounds.x + x, buildingOutline.bounds.y);
			los.CalculateVectors();
		//	cout << "trying at " << buildingOutline.bounds.x + x << endl;
			
			EvaluateLOS(pixels, pointNear);
		
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

		
	EvaluateLOS(pixels, pointNear);

	if (los.score == 0)
		cout << "no symmetry detected (not enough points???)" << endl;
	else
		cout << "Score = " << los.score << endl;

	if (strokes.size() > 0)
		cout << strokes[0].points.size() << " points " << endl;

}


void SymmetryApplication::EvaluateLOS(bool pixels[805][605], bool pointNear[805][605])
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


std::vector<Stroke> SymmetryApplication::ApplyLOS(bool pixels[805][605], bool pointNear[805][605], bool mirrorLeft)
{
	vector<Stroke> outputStrokes;

	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// Prepare first output stroke
		vector<Stroke> generatedStrokes;
		generatedStrokes.push_back(Stroke());
		int currentStroke = 0;

		// Primary side is the side being mirrored, secondary side is the side being discarded
		enum Orientation { PRIMARY, CENTRE, SECONDARY };
		Orientation or = PRIMARY;

		// Start out in greedy mode - TODO explain this
		bool greedyMode = true;

		// Used to store points on primary side in non-greedy mode
		deque<int2> primaryPointsQueue;
		// Used to store generated secondary points in either mode
		stack<int2> secondaryPointsStack;
				


		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			// For current point p:
			float2 p = *it2;
					
			// Calculate magnitude of projection of p (the point's position vector) on LOS
			float proj = los.ProjectedVectorMagnitude(p);
			// Negative if the LOS is in between the origin and p. Positive otherwise
			float pointToLosDistance = proj - los.distanceFromOrigin;
				
			// Below code was written assuming LHS was being mirrored - we can simply invert pointToLosDistance
			// to make it work for mirroring RHS
			if (!mirrorLeft)
				pointToLosDistance *= -1;
	
	
			if (pointToLosDistance > 0)
			{
				// Point is on secondary side

				if ( (or == PRIMARY) || (or == CENTRE) )
				{
					// Transition from primary/centre to secondary
					
					// Flush stack
					// Could be primary points or secondary counterparts, depending on whether we were in greedy mode - we don't care
					generatedStrokes.push_back(CreateStroke(secondaryPointsStack));
					++currentStroke;

					if (!greedyMode)
					{
						// Flush queue which consists of primaries
						generatedStrokes.push_back(CreateStroke(primaryPointsQueue));
						++currentStroke;
					}
											
					greedyMode = false;
				}

				or = SECONDARY;
			}
			else if (pointToLosDistance == 0)
			{
				// Point is on LOS
				or = CENTRE;

				if (greedyMode)
					generatedStrokes[currentStroke].points.push_back(p);
				else
					primaryPointsQueue.push_back(p);
			}
			else
			{
				// Point is on primary side
				or = PRIMARY;
				
				// Calculate position of counterpart point
				float2 counterpartPosition;
				
				if (mirrorLeft)
					counterpartPosition = p + 2 * pointToLosDistance * los.ccwPerp; 
				else
					counterpartPosition = p + 2 * pointToLosDistance * los.cwPerp; 
				
				int2 counterpartPosAsInts(counterpartPosition.x, counterpartPosition.y);


				if (greedyMode)
				{
					generatedStrokes[currentStroke].points.push_back(p);
					secondaryPointsStack.push(counterpartPosAsInts);
				}
				else
				{
					secondaryPointsStack.push(counterpartPosAsInts);
					primaryPointsQueue.push_back(p);
				}
			}
		}


		// We've run out of points - flush the queue + stack
		generatedStrokes.push_back(CreateStroke(secondaryPointsStack));
		++currentStroke;

		if (!greedyMode)
		{
			// Add points in queue to new stroke - these are primaries
			generatedStrokes.push_back(CreateStroke(primaryPointsQueue));
			++currentStroke;
		}

		// Combine strokes if possible
		cout << "Number of output strokes for input stroke = " << generatedStrokes.size() << endl;
		cout << "Can we combine any?" << endl;

		bool successfulCombine = true;;

		while(successfulCombine && generatedStrokes.size() > 1)
		{
			successfulCombine = false;

			for (unsigned stroke = 1; stroke < generatedStrokes.size(); stroke++)
			{
				if (CanMerge(generatedStrokes[stroke - 1], generatedStrokes[stroke]))
				{
					vector<int2>& pr = generatedStrokes[stroke-1].points;
					vector<int2>& sec = generatedStrokes[stroke].points;

					pr.insert(pr.end(), sec.begin(), sec.end());

					generatedStrokes.erase(generatedStrokes.begin() + stroke);

					successfulCombine = true;
					break;
				}
			}
		}
		

		for (vector<Stroke>::iterator it = generatedStrokes.begin(); it < generatedStrokes.end(); it++)
			outputStrokes.push_back(*it);


	}

	// Remove points outside bounds - TODO


	return outputStrokes;
}

bool SymmetryApplication::CanMerge(Stroke& first, Stroke& second)
{
	if (first.points.size() == 0 || second.points.size() == 0)
		return true;


	int2 disp = second.points[0] - first.points[first.points.size() - 1];
	int dist = sqrt((float) dot(disp, disp));

	cout << "\tdist = " << dist << endl;
	return (dist < 50);
}


Stroke SymmetryApplication::CreateStroke(deque<int2>& points)
{
	Stroke str;

	while(!points.empty())
	{
		str.points.push_back(points.front());
		points.pop_front();
	}

	return str;
}

Stroke SymmetryApplication::CreateStroke(stack<int2>& points)
{
	Stroke str;

	while(!points.empty())
	{
		str.points.push_back(points.top());
		points.pop();
	}

	return str;
}
				


LineOfSymmetry SymmetryApplication::GetLOS()
{
	return los;
}

bool SymmetryApplication::LOSIsValid()
{
	return los.score > 0;
}