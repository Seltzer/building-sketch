// Main algorithms for generating a Building from sketches and feature data
#ifndef BUILDING_GENERATION_H_
#define BUILDING_GENERATION_H_



#include <vector>
#include "Common.h"


void ExtrudeSketch(Building& building, std::vector<int2>& outline, std::vector<Stroke>& processedFeaturesOutline);

void RotateSketch(Building& building, std::vector<int2>& outline, int rotationCount, bool mirrorSketch);

void MirrorSketch(Building& building, std::vector<int2>& outline);










#endif