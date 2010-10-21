#include <iostream>
#include "DisplacementMapping.h"

sf::Image displacementMap;
// The displacement vector stores the image data as it is being processed
std::vector<std::vector<PixelData>> displacementVector;

sf::Color black = sf::Color(0, 0, 0, 255);
sf::Color gray = sf::Color(128, 128, 128, 255);
sf::Color white = sf::Color(255, 255, 255, 255);
sf::Color red = sf::Color(255, 0, 0, 255);
sf::Color blue = sf::Color(0, 0, 255, 255);
sf::Color green = sf::Color(0, 255, 0, 255);
sf::Color yellow = sf::Color(255, 255, 0, 255);
sf::Color magenta = sf::Color(255, 0, 255, 255);
sf::Color cyan = sf::Color(0, 255, 255, 255);
sf::Color purple = sf::Color(163, 73, 163, 255);
sf::Color lightgray = sf::Color(200, 200, 200, 255);

int width;
int height;

void generateDisplacementMap(Bounds bounds, std::vector<Stroke>& featureStrokes)
{
	if (featureStrokes.empty()) return;

	// Set the width and heigth of the map.
	width = bounds.width;
	height = bounds.height;

	// Set up the displacement vector.
	std::vector<PixelData> yVector;
	PixelData pixelData;
	pixelData.color = white;
	pixelData.strokeID = -1;
	pixelData.intersectingLines = false;
	yVector.reserve(height);
	yVector.assign(height, pixelData);
	displacementVector.reserve(width);
	displacementVector.assign(width, yVector);

	// Set up the displacement image.
	displacementMap.Create(width, height, white);

	for (int i = 0; i <= featureStrokes.size()-1; i++)
	{
		Stroke stroke = featureStrokes[i];
		//Stroke stroke = (*strokeItterator);
		int2 oldPoint = stroke.points[0];
		for (int j = 0; j <= stroke.points.size()-1; j++)
		{			
			int2 point = stroke.points[j];
			plotLine(point.x, oldPoint.x, point.y, oldPoint.y, i, j);
			oldPoint = point;
		}
		fillPloy(stroke, i);		
	}

	vectorToImage();
	displacementMap.SaveToFile("displacement_map.png");
}

void plotLine(int x0, int x1, int y0, int y1, int strokeID, int lineID)
{	
	bool steep = abs(y1-y0) > abs(x1-x0);
	if (steep) 
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error = deltax / 2;
	int ystep;
	int y = y0;
	ystep = (y0 < y1) ? 1 : -1;
	for (int x = x0; x <= x1; x++) 
	{
		if (steep)
		{
			if ((displacementVector[y][x].color == black) && (displacementVector[y][x].strokeID == strokeID))
			{
				displacementVector[y][x].intersectingLines = true;
			}
			displacementVector[y][x].color = black;
			displacementVector[y][x].strokeID = strokeID;	
			displacementVector[y][x].lineID.push_back(lineID);
		} 
		else
		{
			if ((displacementVector[x][y].color == black) && (displacementVector[x][y].strokeID == strokeID))
			{
				displacementVector[x][y].intersectingLines = true;				
			}
			displacementVector[x][y].color = black;
			displacementVector[x][y].strokeID = strokeID;
			displacementVector[x][y].lineID.push_back(lineID);
		}
		error = error - deltay;
		if (error < 0)
		{
			y = y + ystep;
			error = error + deltax;
		}
	}
}

void fillPloy(Stroke stroke, int strokeID)
{
	std::cout << "FILL POLY ID: " << strokeID << std::endl << "====================================================" << std::endl << std::endl;
	Bounds bounds = stroke.bounds;
	bool shouldFill;
	bool noChange;
	bool flatLineCheck;
	bool flatPeakCheck;
	std::vector<int> lastFlipLine;

	for (int y = bounds.y+1; y <= (bounds.y+bounds.height-1); y++)
	{
		flatLineCheck = false;
		flatPeakCheck = false;
		shouldFill = false;
		noChange = false;
		if (displacementMap.GetPixel(bounds.x,y) == black) shouldFill = true;
		lastFlipLine.clear();
		for (int x = bounds.x; x <= (bounds.x+bounds.width); x++)
		{		
			// If this pixel is part of the feature stroke
			if (displacementVector[x][y].strokeID == strokeID)
			{
				// If this pixel is an edge
				if (displacementVector[x][y].color == black)
				{									
					// If this pixel is part of 2 or more lines
					if (displacementVector[x][y].intersectingLines)
					{
						// If this pixel is at a spike (see isSpike)
						if (isSpike(x, y, stroke, strokeID))
						{
							displacementVector[x][y].color = yellow;
							if (noChange) {
								// Check for a flat line
								if ((!flatPeakCheck) && (!flatLineCheck))shouldFill = !shouldFill;
								displacementVector[x][y].color = blue;
								flatLineCheck = !flatLineCheck;
								flatPeakCheck = false;
							} 
							else
							{
								displacementVector[x][y].color = red;
								flatPeakCheck = true;
							}								
							noChange = true;
						}
						else
						{	
							displacementVector[x][y].color = lightgray;
							if ((!noChange) || (!contains(displacementVector[x-1][y].lineID, displacementVector[x][y].lineID)))
							{
								if (!isIntersectingAround(x,y))
								{
									displacementVector[x][y].color = magenta;
									shouldFill = !shouldFill;
									noChange = true;
								} else
								{
									displacementVector[x][y].color = cyan;
									if (noChange)
										shouldFill = !shouldFill;
									noChange = true;
								}
							}
						}
					}
					else
					{
						if ((!noChange) || (!contains(displacementVector[x-1][y].lineID, displacementVector[x][y].lineID)))
						{
							displacementVector[x][y].color = purple;
							shouldFill = !shouldFill;
							noChange = true;
						}
					}
				}		
			}
			else // If this pixel is not an edge.
			{
				noChange = false;
				flatPeakCheck = false;
				flatLineCheck = false;
			}
			// If the pixel should be filled.
			if (shouldFill)
			{
				if (displacementVector[x][y].color == white)
				{
					displacementVector[x][y].color = gray;
				}
			}
		}
	}
}
		
// TODO: Change this to a bit flag instead of vector.
bool contains(std::vector<int> v1, std::vector<int> v2)
{
	if (v1.empty()) return false;
	for (std::vector<int>::iterator vectorIterator1 = v1.begin(); vectorIterator1 != v1.end(); vectorIterator1++)
	{
		int number1 = (*vectorIterator1);
		for (std::vector<int>::iterator vectorIterator2 = v2.begin(); vectorIterator2 != v2.end(); vectorIterator2++)
		{
			int number2 = (*vectorIterator2);
			if (number1 == number2)	return true;
		}
	}
	return false;
}

bool isIntersectingAround(int x, int y)
{
	// TODO: check for pixels out of bounds
	//		 re-write this function. yuk :(

	std::vector<int> topLineID = displacementVector[x][y+1].lineID;
	std::vector<int> topLeftLineID = displacementVector[x-1][y+1].lineID;
	std::vector<int> topRightLineID = displacementVector[x+1][y+1].lineID;

	std::vector<int> bottomLineID = displacementVector[x][y-1].lineID;
	std::vector<int> bottomLeftLineID = displacementVector[x-1][y-1].lineID;
	std::vector<int> bottomRightLineID = displacementVector[x+1][y-1].lineID;

	if ((displacementVector[x][y+1].intersectingLines) || 
		(displacementVector[x-1][y+1].intersectingLines) || 
		(displacementVector[x+1][y+1].intersectingLines) || 
		(displacementVector[x][y-1].intersectingLines) || 
		(displacementVector[x-1][y-1].intersectingLines) || 
		(displacementVector[x+1][y-1].intersectingLines))
	return true;
	
	if ((!contains(topLineID, topLeftLineID))
		&& (displacementVector[x][y+1].color == black)
		&& (displacementVector[x-1][y+1].color == black)
		)
	return true;

	if ((!contains(topLineID, topRightLineID))
		&& (displacementVector[x][y+1].color == black)
		&& (displacementVector[x+1][y+1].color == black)
		)
	return true;

	if ((!contains(topRightLineID, topLeftLineID))
		&& (displacementVector[x-1][y+1].color == black)
		&& (displacementVector[x+1][y+1].color == black)
		)
	return true;

	if ((!contains(bottomLineID, bottomLeftLineID))
		&& (displacementVector[x][y-1].color == black)
		&& (displacementVector[x-1][y-1].color == black)
		)
		return true;

	if ((!contains(bottomLineID, bottomRightLineID))
		&& (displacementVector[x][y-1].color == black)
		&& (displacementVector[x+1][y-1].color == black)
		)
		return true;

	if ((!contains(bottomRightLineID, bottomLeftLineID))
		&& (displacementVector[x-1][y-1].color == black)
		&& (displacementVector[x+1][y-1].color == black)
		)
		return true;

	return false;
}

/*
 * Check if this point is a spike.
 * ie. is it a pointy bit. (angle less than or equal to 90degrees)
 *
 * return true if it is a spike.
 * return false if it is not.
 */
bool isSpike(int x, int y, Stroke stroke, int pointID1)
{
	//TODO FIX!
	int pointPos = -1;
	int leftY = y, rightY = y;

	// Find which vertex this is. There will be only one
	// as per the nature of recording them.
	for (int i = 0; i <= stroke.points.size()-1; i++) 
	{
		int2 point = stroke.points[i];
		if ((point.x == x) && (point.y == y))
		{
			pointPos = i;
			break;
		}
	}
	if (pointPos == -1) return false;

	// Get the left and right points. If the left or right points are at the exact same
	// y value, get the next point. We do not want to deal with horizontal lines. They are evil.
	int newLeftPos = pointPos;
	while (leftY == y)
	{
		int prevPointPos = (newLeftPos == 0) ? stroke.points.size()-2 : newLeftPos-1;
		leftY = stroke.points[prevPointPos].y;
		newLeftPos--;
	}
	int newRightPos = pointPos;
	while (rightY == y)
	{
		int nextPointPos = (newRightPos == stroke.points.size()-1) ? 1 : newRightPos+1;
		rightY = stroke.points[nextPointPos].y;
		newRightPos++;
	}
	
	if (((leftY >= y) && (rightY <= y))
	   || ((leftY <= y) && (rightY >= y)))
	return false;
	return true;
}

/*
 * Copies appropriate pixel data from the vector to the image.
 */
void vectorToImage()
{
	for (int x = 0; x <= width-1; x++) 
	{
		for (int y = 0; y <= height-1; y++) 
		{
			displacementMap.SetPixel(x, y, displacementVector[x][y].color);
		}
	}
}