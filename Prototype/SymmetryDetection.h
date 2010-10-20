#ifndef SYMMETRY_DETECTION_H_
#define SYMMETRY_DETECTION_H_


#include <vector>
#include "Types.h"
#include "Common.h"



// This currently calculates the best fitting line of symmetry and does random stuff (TODO either mirror one half of the sketch or average both halves)
//		Note: mirroredStroke1 and mirroredStroke2 are output parameters used for testing purposes
LineOfSymmetry CalculateSymmetry(bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2);

// Evaluate a line of symmetry, and return a heuristic score
void EvaluateLOS(LineOfSymmetry& los, bool pixels[805][605], bool pointNear[805][605], std::vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2);

bool PointExistsNear(bool pixels[805][605], bool pointNear[805][605], const int2 position);

bool EdgeExistsBetween(bool pixels[805][605], bool pointNear[805][605], const int2, const int2);

















#endif