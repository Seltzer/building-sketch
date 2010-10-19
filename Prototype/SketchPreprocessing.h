#ifndef SKETCH_PREPROCESSING_H_
#define SKETCH_PREPROCESSING_H_

#include <vector>
#include "Types.h"
#include "Common.h"


std::vector<int2> DouglasPeuker(std::vector<int2>::const_iterator start, std::vector<int2>::const_iterator end, float threshold);
Stroke Reduce(const Stroke& stroke, float threshold);
// This currently calculates the best fitting line of symmetry and does random stuff (TODO either mirror one half of the sketch or average both halves)
//		Note: mirroredStroke1 and mirroredStroke2 are output parameters used for testing purposes
LineOfSymmetry CalculateSymmetry(std::vector<Stroke>& strokes, Stroke& buildingOutline, Stroke& mirroredStroke1, Stroke& mirroredStroke2);




#endif