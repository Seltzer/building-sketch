#include <iostream>
#include <SFML/Graphics.hpp>
#include <math.h>
#include "DisplacementMapping.h"

sf::Image displacementMap;
sf::Color black = sf::Color(0, 0, 0, 255);
sf::Color gray = sf::Color(128, 128, 128, 255);
sf::Color white = sf::Color(255, 255, 255, 255);


void generateDisplacementMap(Bounds bounds, std::vector<Stroke>& featureStrokes)
{
	//displacementMap;
	displacementMap.Create(bounds.width, bounds.height, white);

	for (std::vector<Stroke>::const_iterator strokeItterator = featureStrokes.begin(); strokeItterator != featureStrokes.end(); strokeItterator++)
	{
		Stroke stroke = (*strokeItterator);
		int2 oldPoint = stroke.points[0];
		for (std::vector<int2>::const_iterator v = stroke.points.begin()+1; v != stroke.points.end(); v++)
		{			
			int2 point = (*v);
			linePlot(point.x, oldPoint.x, point.y, oldPoint.y);
			oldPoint = point;
		}
		ployFill(bounds, stroke.bounds);
	}

	bool success = displacementMap.SaveToFile("displacement_map.jpg");
}

void linePlot(int x0, int x1, int y0, int y1)
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
			displacementMap.SetPixel(y,x, black);
		} 
		else
		{
			displacementMap.SetPixel(x,y, black);
		}
		error = error - deltay;
		if (error < 0)
		{
			y = y + ystep;
			error = error + deltax;
		}
	}
}

void ployFill(Bounds buildingBounds, Bounds strokeBounds)
{
	Bounds bounds = strokeBounds;
	bool shouldFill;

	std::cout << "x: " << bounds.x << std::endl;
	std::cout << "y: " << bounds.y << std::endl;
	std::cout << "width: " << bounds.width << std::endl;
	std::cout << "height: " << bounds.height << std::endl;
	std::cout << "building width: " << buildingBounds.width << std::endl;
	std::cout << "building height: " << buildingBounds.height << std::endl;


	for (int y = bounds.y; y <= (bounds.y+bounds.height); y++)
	{
		shouldFill = false;
		//if (displacementMap.GetPixel(bounds.x,y) == black) shouldFill = true;
		for (int x = bounds.x; x <= (bounds.x+bounds.width); x++)
		{		
			if (displacementMap.GetPixel(x,y) == black)
			{
				if (displacementMap.GetPixel(x-1,y) != black)
					shouldFill = !shouldFill;
			}
			if ((shouldFill) && (displacementMap.GetPixel(x,y) == white)) displacementMap.SetPixel(x,y, gray);
		}
	}
}