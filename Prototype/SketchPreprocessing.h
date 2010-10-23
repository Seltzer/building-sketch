#ifndef SKETCH_PREPROCESSING_H_
#define SKETCH_PREPROCESSING_H_

#include <vector>
#include "Types.h"
#include "Common.h"


std::vector<int2> DouglasPeuker(std::vector<int2>::const_iterator start, std::vector<int2>::const_iterator end, float threshold);
Stroke Reduce(const Stroke& stroke, float threshold);

unsigned NumberOfPoints(std::vector<Stroke>&);


// Fuzziness threshold affects the interpretation of 'near'
template<int WIDTH, int HEIGHT>
void AddPointToMatrices(bool pixels[WIDTH][HEIGHT], bool pixelsNear[WIDTH][HEIGHT], int fuzzinessThreshold, int2& point);

template<int WIDTH, int HEIGHT>
void StrokesToMatrices(bool pixels[WIDTH][HEIGHT], bool pixelsNear[WIDTH][HEIGHT], int fuzzinessThreshold, std::vector<Stroke>& strokes);



#include "SketchPreprocessing.inl"

#endif