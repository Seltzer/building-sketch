#ifndef BUILDINGSKETCH_H
#define BUILDINGSKETCH_H

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "Types.h"
#include "Clip.h"
#include "Concave.h"

class BuildingSketch
{
public:	
	typedef std::vector<float3> Poly;
	struct Stroke
	{
		std::vector<int2> points;
	};

	struct BoundingBox
	{
		int3 min;
		int3 max;
	};

	struct Building
	{
		//BoundingBox bounds;
		std::vector<Poly> polys;
	};
	
	BuildingSketch();
	std::vector<int2> DouglasPeuker(std::vector<int2>::const_iterator start, std::vector<int2>::const_iterator end, float threshold);
	Stroke Reduce(const Stroke& stroke, float threshold);
	void DrawStroke(const Stroke& stroke);
	float randFloat();
	void RandomColor();
	void DrawOutline(const Poly poly);
	void UpdateBuilding();
	void Process(const Stroke& stroke);
	void MousePressed();
	void MouseReleased();
	void MouseMoved(int2 pos);
	void RenderLines();
	void RenderBuilding();
	void RenderLoop();	

private:
	sf::RenderWindow* win;

	Stroke currentStroke;
	std::vector<Stroke> strokes;
	std::vector<Stroke> polyLines;

	Building building;

	bool filled;
	bool drawing;
	float yaw;
	float pitch;
};

#endif //BUILDINGSKETCH_H