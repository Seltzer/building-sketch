#ifndef SKETCH_PREPROCESSING_H_
#define SKETCH_PREPROCESSING_H_

#include <vector>
#include "Types.h"
#include "Common.h"


std::vector<int2> DouglasPeuker(std::vector<int2>::const_iterator start, std::vector<int2>::const_iterator end, float threshold);
Stroke Reduce(const Stroke& stroke, float threshold);




#endif