#include <iostream>
#include "DisplacementMapping.h"

sf::Image displacementMap;
// The displacement vector stores the image data as it is being processed
std::vector<std::vector<PixelData>> displacementVector;

sf::Color black = sf::Color(0, 0, 0, 255);
sf::Color gray = sf::Color(128, 128, 128, 255);
sf::Color white = sf::Color(255, 255, 255, 255);
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
		fillPloy(stroke.bounds, i);		
	}

	vectorToImage();
	displacementMap.SaveToFile("displacement_map.jpg");
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

void fillPloy(Bounds strokeBounds, int strokeID)
{
	Bounds bounds = strokeBounds;
	bool shouldFill;
	std::vector<int> lastFlipLine;

	for (int y = bounds.y+1; y <= (bounds.y+bounds.height-1); y++)
	{
		shouldFill = false;
		if (displacementMap.GetPixel(bounds.x,y) == black) shouldFill = true;
		lastFlipLine.clear();
		for (int x = bounds.x; x <= (bounds.x+bounds.width); x++)
		{		
			if (displacementVector[x][y].color == black)
			{
				if ((displacementVector[x][y].strokeID == strokeID)
					&& (!contains(displacementVector[x][y].lineID, lastFlipLine))
					&& (displacementVector[x-1][y].color != black)
					)
				{
					if ((!displacementVector[x][y].intersectingLines)
						|| ((!isPeak(x,y))
						&& (!isIntersectingAround(x,y))
						)) // && (displacementVector[x-1][y].color != black) /*&& (!displacementVector[x][y].intersectingLines)*/
					{
						shouldFill = !shouldFill;
						
					}lastFlipLine = displacementVector[x][y].lineID;
				}
			}
			if ((shouldFill) && (displacementVector[x][y].color == white)) displacementVector[x][y].color = gray;
		}
	}
}

// TODO: Change this to a bit flag instead of vector. Bah humbug!
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
	bool isIntersecting = false; 
	if ((displacementVector[x][y+1].intersectingLines) || 
		(displacementVector[x-1][y+1].intersectingLines) || 
		(displacementVector[x+1][y+1].intersectingLines) || 
		(displacementVector[x][y-1].intersectingLines) || 
		(displacementVector[x-1][y-1].intersectingLines) || 
		(displacementVector[x+1][y-1].intersectingLines))
		isIntersecting = true;
	return isIntersecting;
}

/*
 * Check if this point is a peak or trough
 * ie. it is not connected to another line either above or below it.
 * and neither on its left nor right.
 */
bool isPeak(int x, int y)
{
	bool isPeak = false;
	// Check above
	if ((displacementVector[x][y-1].color != black)
		&& (displacementVector[x-1][y-1].color != black)
		&& (displacementVector[x+1][y-1].color != black))
		isPeak = true;
	// Check below
	if ((displacementVector[x][y+1].color != black)
		&& (displacementVector[x-1][y+1].color != black)
		&& (displacementVector[x+1][y+1].color != black))
		isPeak = true;
	return isPeak;
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