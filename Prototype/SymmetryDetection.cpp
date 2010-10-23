#include <iostream>
#include <map>

#include "SymmetryDetection.h"
#include "SketchPreprocessing.h"




using namespace std;




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

		if (!pointNear[805][605])
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
	: pointOnLine(0,0), direction(0,0), ccwPerp(0,0), cwPerp(0,0), distanceFromOrigin(0), matches(INVALID), score(INVALID), numberOfPointsBasedOn(INVALID)
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

void LineOfSymmetry::UpdateStats(int matches, int score, int numberOfPointsBasedOn)
{
	this->matches = matches;
	this->score = score;
	this->numberOfPointsBasedOn = numberOfPointsBasedOn;
}

string LineOfSymmetry::ToString()
{
	stringstream ss;
	ss << "********** pointOnLine = " << pointOnLine.tostring() << "\n";
	ss << "********** direction = " << direction.tostring() << "\n";
	ss << "********** ccwPerp = " << ccwPerp.tostring() << "\n";
	ss << "********** cwPerp = " << cwPerp.tostring() << "\n";
	ss << "********** #matches = " << matches << "\n";
	ss << "********** Score = " << score << "\n";
	ss << "********** Number of sketch points this was based on = " << numberOfPointsBasedOn << endl;

	return ss.str();
}





//////////////////////////////////////////////////////////// SymmetryApplication Implementation
SymmetryApplication::SymmetryApplication(Stroke& buildingOutline)
	: buildingOutline(buildingOutline), usingInterpolation(false)
{
	buildingOutline.CalculateBounds();
}


void SymmetryApplication::CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605], vector<Stroke>& strokes)
{
	LineOfSymmetry candidateLOS;
	bestLOS = candidateLOS;
	

	// TODO check that this tries 0
	for (float deltaX = -0.4; deltaX <= 0.4; deltaX += 0.05)
	{
		cout << "deltaX = " << deltaX << endl;
		candidateLOS.direction = float2(deltaX,1);
		
		for (int x = 0; x < buildingOutline.bounds.width; x++)
		{
			candidateLOS.pointOnLine = int2(buildingOutline.bounds.x + x, buildingOutline.bounds.y);
			candidateLOS.CalculateVectors();
			
			if (EvaluateLOS(candidateLOS, pixels, pointNear, strokes) > bestLOS.score)
				bestLOS = candidateLOS;
		}
	}


	if (bestLOS.score == 0)
		cout << "no symmetry detected (not enough points???)" << endl;
	else
		cout << bestLOS.ToString() << endl;
}

void SymmetryApplication::CalculateSymmetry()
{
	assert(usingInterpolation);
	return CalculateSymmetry(interpolatedPixels, interpolatedPixelsNear, interpolatedStrokes);
}


void SymmetryApplication::CalculateSymmetry(vector<Stroke>& strokes)
{
	assert(usingInterpolation);
	return CalculateSymmetry(interpolatedPixels, interpolatedPixelsNear, strokes);
}

int SymmetryApplication::EvaluateLOS(LineOfSymmetry& los, bool pixels[805][605], bool pointNear[805][605], vector<Stroke>& strokes)
{
	// Number of pair matches found so far
	unsigned matches = 0;
	// Mapping midpoints to number of matches (this is used to mitigate effects of edges which have many matches
	map<int2,int> midpointCount;
	// Mapping distances to number of matches (used to measure spread of matches around LOS)
	map<int,int> matchDistribution;
	// Max distance a match point is from LOS. Number of points in sketch input
	unsigned maxDistance = 0, sketchPoints = 0;


	// Iterate over all strokes
	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			// For current point p:
			float2 p = *it2;
			sketchPoints++;
					
			// Calculate magnitude of projection of p (the point's position vector) on LOS perpendicular
			float proj = los.ProjectedVectorMagnitude(p);

			// Negative if the LOS is in between the origin and p. Positive otherwise
			float pointToLosDistance = proj - los.distanceFromOrigin;
			

			// Calculate vector between point and LOS
			float2 vectorToLOS = pointToLosDistance * los.ccwPerp;

			// Calculate midpoint and counterpart
			float2 midpointPosition = p + vectorToLOS;	//cout << "\t midpointPosition = " << midpointPosition.tostring() << " / " << originToLosDistance << endl;
			float2 counterpartPosition = midpointPosition + vectorToLOS;
			int2 counterpartPosAsInts(counterpartPosition.x, counterpartPosition.y);


			if ( (counterpartPosAsInts.x >= 0) && (counterpartPosAsInts.x <= 800) )
			{
				if ( (counterpartPosAsInts.y >= 0) && (counterpartPosAsInts.y <= 600) )
				{
					// This is true when p lies on the line of symmetry - this will result in a match which we should ignore
					if (p == counterpartPosAsInts)
						continue;

					//if (PointExistsNear(pixels, pointNear, counterpartPosAsInts))
					if (pointNear[counterpartPosAsInts.x][counterpartPosAsInts.y])
					{
						int dist = pointToLosDistance;
	
						// Increment count of matches centered on this midpoint			
						int2 midpointAsInts(midpointPosition.x, midpointPosition.y);
						midpointCount[midpointAsInts]++;

						// If there are already >4 matches centered on this midpoint, then there's a good chance we're dealing
						// with an edge... try to determine this
						if (midpointCount[midpointAsInts] > 4)
						{
							if (EdgeExistsBetween(pixels, pointNear, p, counterpartPosAsInts))
								continue;
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
			cout << "spread = " << spread << " and maxSpread = " << maxSpread << endl;
			cout << "coefficient = " << coefficient << endl;
		#endif
	}
	
	los.UpdateStats(matches, matches * coefficient, sketchPoints);

	#ifdef DEBUGGING
		cout << los.ToString() << endl;
	#endif

	return los.score;
}


// TODO Ignore points outside bounds
std::vector<Stroke> SymmetryApplication::ApplyLOS(bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>& strokes, bool mirrorLeft)
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
		// TODO this is wrong
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
			float proj = bestLOS.ProjectedVectorMagnitude(p);
			// Negative if the LOS is in between the origin and p. Positive otherwise
			float pointToLosDistance = proj - bestLOS.distanceFromOrigin;
				
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
					
					// Flush stack - it contains either primary points or secondary counterparts, depending on whether we're in greedy mode
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
					counterpartPosition = p + 2 * pointToLosDistance * bestLOS.ccwPerp; 
				else
					counterpartPosition = p + 2 * pointToLosDistance * bestLOS.cwPerp; 
				
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
			// Flush queue which contains primaries
			generatedStrokes.push_back(CreateStroke(primaryPointsQueue));
			++currentStroke;
		}

		// Combine strokes if possible
		CombineStrokes(generatedStrokes);
		
		// And add to master output
		for (vector<Stroke>::iterator it = generatedStrokes.begin(); it < generatedStrokes.end(); it++)
			outputStrokes.push_back(*it);
	}


	return outputStrokes;
}

void SymmetryApplication::CombineStrokes(vector<Stroke>& strokes)
{
	cout << "Number of output strokes for input stroke = " << strokes.size() << endl;
	cout << "Can we combine any?" << endl;

	// Remove any strokes with no points
	for (int index = strokes.size() - 1; index >= 0; index--)
	{
		if (strokes[index].points.size() == 0)
			strokes.erase(strokes.begin() + index);
	}


	// Try to combine remaining strokes
	bool successfulCombine = true;;

	while(successfulCombine && strokes.size() > 1)
	{
		successfulCombine = false;

		for (unsigned stroke = 1; stroke < strokes.size(); stroke++)
		{
			if (CanMerge(strokes[stroke - 1], strokes[stroke]))
			{
				vector<int2>& pr = strokes[stroke-1].points;
				vector<int2>& sec = strokes[stroke].points;

				pr.insert(pr.end(), sec.begin(), sec.end());

				strokes.erase(strokes.begin() + stroke);

				successfulCombine = true;
				break;
			}
		}
	}
}

bool SymmetryApplication::CanMerge(Stroke& first, Stroke& second, unsigned threshold)
{
	if (first.points.size() == 0 || second.points.size() == 0)
		return true;


	int2 disp = second.points[0] - first.points[first.points.size() - 1];
	int dist = sqrt((float) dot(disp, disp));

	cout << "\tdist = " << dist << endl;
	return (dist < threshold);
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
				
void SymmetryApplication::InterpolateSketch(vector<Stroke>& inputStrokes)
{
	StrokesToMatrices<805, 605>(interpolatedPixels, interpolatedPixelsNear, 1, inputStrokes);

	// Debugging
	int interpCount = 0;
	int inputPoints = 0;
	int resultingPoints = 0;


	// Iterate over all strokes
	for (vector<Stroke>::iterator it = inputStrokes.begin(); it < inputStrokes.end(); it++)
	{
		if ((*it).points.size() == 0)
			continue;

		Stroke interpolatedStroke;

		int2 lastPoint = (*it).points[0];

		// Iterate over points of each stroke
		for (vector<int2>::iterator it2 = (*it).points.begin(); it2 < (*it).points.end(); it2++)
		{
			int2& point = *it2;
			++inputPoints;
		
			// Guard against self-interpolation
			if (point == lastPoint)
				continue;

			float2 diff = point - lastPoint;
			lastPoint = point;

			float length = sqrt((float) dot(diff, diff));
			if (length <= 0)
				continue;

			
			float incr = 1 / length;

			// Move parametrically along diff vector, interpolating points along the way
			for (float t = 0; t <= 1; t += incr)
			{
				int2 interpPoint = lastPoint + t * diff;

				if (!interpolatedPixels[interpPoint.x][interpPoint.y])
				{
					++interpCount;

					AddPointToMatrices<805, 605>(interpolatedPixels, interpolatedPixelsNear, 1, interpPoint);
					interpolatedStroke.points.push_back(interpPoint);
				}
			}
		
			interpolatedStroke.points.push_back(point);
		}

		interpolatedStrokes.push_back(interpolatedStroke);
	}



	for (int x = 0; x < 805; x++)
	{
		for (int y = 0; y < 605; y++)
		{
			if (interpolatedPixels[x][y])
				resultingPoints++;
		}
	}


	cout << "interpCount = " << interpCount << endl;
	cout << "inputPoints = " << inputPoints << endl;
	cout << "resultingPoints = " << resultingPoints << endl;

	usingInterpolation = true;
}


LineOfSymmetry SymmetryApplication::GetLOS()
{
	return bestLOS;
}

bool SymmetryApplication::LOSIsValid()
{
	return bestLOS.score > 0;
}