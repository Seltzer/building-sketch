#ifndef SYMMETRY_DETECTION_H_
#define SYMMETRY_DETECTION_H_


#include <vector>
#include <deque>
#include <stack>
#include "Types.h"
#include "Common.h"


// Utility functions
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

	int matches, score, numberOfPointsBasedOn;	


	LineOfSymmetry();

	// Normalises direction and calculates ccwPerp, cwPerp and distanceFromOrigin
	void CalculateVectors();

	// Project a vector along the line given by ccwPerp/cwPerp, and return the magnitude of the resulting vector
	float ProjectedVectorMagnitude(float2 vec);

	void UpdateStats(int matches, int score, int numberOfPointsBasedOn);

	// For debugging
	std::string ToString();
};



// Class which calculates a line of symmetry, and mirrors one side of an existing sketch according to the LOS
class SymmetryApplication
{

public:
	SymmetryApplication(Stroke& buildingOutline);

	// Interpolation makes symmetry detection slower but more accurate... should probably only be called
	// if there aren't many points in the sketch input
	void InterpolateSketch(std::vector<Stroke>&);


	// This calculates the best fitting line of symmetry
	void CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>&);

	// Overload which uses interpolated pixels/strokes as counterparts (but not primaries)
	// Pre-Condition: InterpolateSketch has been called
	void CalculateSymmetry(std::vector<Stroke>&);

	// Overload which uses interpolated pixels/strokes as both primaries and counterparts
	// Pre-Condition: InterpolateSketch has been called
	void CalculateSymmetry();


	// Apply last calculated LOS to sketch by mirroring one half and discarding the other
	std::vector<Stroke> ApplyLOS(bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>&, bool mirrorLeft);

	LineOfSymmetry GetLOS();
	bool LOSIsValid();
	
private:
	int EvaluateLOS(LineOfSymmetry& los, bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>&);

	Stroke CreateStroke(std::deque<int2>&);
	Stroke CreateStroke(std::stack<int2>&);

	void CombineStrokes(std::vector<Stroke>&);
	bool CanMerge(Stroke& first, Stroke& second, unsigned threshold = 50);
	

	// Interpolated sketch
	bool usingInterpolation;
	bool interpolatedPixels[805][605];
	bool interpolatedPixelsNear[805][605];
	std::vector<Stroke> interpolatedStrokes;
		

	Stroke buildingOutline;
	LineOfSymmetry bestLOS;
};













#endif