#ifndef DISPLACEMENT_MAPPING
#define DISPLACEMENT_MAPPING

#include <vector>
#include "Types.h"
#include "Common.h"

extern sf::Image displacementMap;

void generateDisplacementMap(Bounds bounds, std::vector<Stroke>& strokes);
void linePlot(int x0, int x1, int y0, int y1);
void ployFill(Bounds buildingBounds, Bounds strokeBounds);

#endif //DISPLACEMENT_MAPPING