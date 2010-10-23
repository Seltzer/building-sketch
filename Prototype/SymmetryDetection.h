#ifndef SYMMETRY_DETECTION_H_
#define SYMMETRY_DETECTION_H_


#include <vector>
#include <deque>
#include <stack>
#include "Types.h"
#include "Common.h"


// Utility functions
bool PointExistsNear(bool pixels[805][605], bool pointNear[805][605], const int2 position);
bool EdgeExistsBetween(bool pixels[805][605], bool pointNear[805][605], const int2, const int2);




// Struct representing a line of symmetry calculated from input strokes, and its score.
struct LineOfSymmetry
{
	int2 pointOnLine;
	// Unit-vector for direction
	float2 direction;
	// Unit vectors for perpendicular vectors (direction rotated CCW and CW by 0.5pi)
	float2 ccwPerp, cwPerp;

	float distanceFromOrigin;

	unsigned matches;	
	int score;


	LineOfSymmetry();

	// Normalises direction and calculates ccwPerp, cwPerp and distanceFromOrigin
	void CalculateVectors();

	// Project a vector along the line given by ccwPerp/cwPerp, and return the magnitude of the resulting vector
	float ProjectedVectorMagnitude(float2 vec);

	// For debugging
	std::string ToString();
};



// Class which calculates a line of symmetry, and mirrors one side of an existing sketch according to the LOS
class SymmetryApplication
{

public:
	SymmetryApplication(std::vector<Stroke>& strokes, Stroke& buildingOutline);

	// This currently calculates the best fitting line of symmetry and does random stuff (TODO either mirror one half of the sketch or average both halves)
	void CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605]);

	// Apply last calculated LOS to sketch, mirror one half and discard the other
	std::vector<Stroke> ApplyLOS(bool pixels[805][605], bool pointNear[805][605], bool mirrorLeft);

	LineOfSymmetry GetLOS();
	bool LOSIsValid();
	
private:
	// Evaluate los
	void EvaluateLOS(bool pixels[805][605], bool pointNear[805][605]);

	Stroke CreateStroke(std::deque<int2>&);
	Stroke CreateStroke(std::stack<int2>&);

	bool CanMerge(Stroke& first, Stroke& second);


	std::vector<Stroke>& strokes;
	Stroke& buildingOutline;

	LineOfSymmetry los;
};













#endif