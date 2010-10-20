#ifndef DISPLACEMENT_MAPPING
#define DISPLACEMENT_MAPPING

#include <vector>
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Types.h"
#include "Common.h"

extern sf::Image displacementMap;

struct PixelData
{
	sf::Color color;
	int strokeID;
	std::vector<int> lineID;
	bool intersectingLines;
};

void generateDisplacementMap(Bounds bounds, std::vector<Stroke>& strokes);
void plotLine(int x0, int x1, int y0, int y1, int strokeID, int lineID);
void fillPloy(Bounds strokeBounds, int strokeID);
bool contains(std::vector<int> v1, std::vector<int> v2);
bool isIntersectingAround(int x, int y);
bool isPeak(int x, int y);
void vectorToImage();

#endif //DISPLACEMENT_MAPPING