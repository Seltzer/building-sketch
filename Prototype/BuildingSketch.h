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
	enum MOUSE_ACTION { NONE, DRAWING, TRACKING};
	enum BUILDING_ALGORITHM { EXTRUDE, ROTATE, MIRROR};

	struct Bounds
	{
		int x;
		int y;
		int width;
		int height;
		int depth;
	};

	struct Stroke
	{
		int length;
		Bounds bounds;
		std::vector<int2> points;
	};

	struct Building
	{
		Bounds bounds;
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
	void ProcessStroke(const Stroke& stroke);
	void ResetStrokes();
	void MousePressed(int2 pos);
	void MouseReleased(int2 pos);
	void MouseMoved(int2 pos);
	void MouseWheelMoved(int delta);
	float3 trackBallMapping(float2 point);
	void RenderLines();
	void RenderBuilding();
	void RenderLoop();	

private:
	sf::RenderWindow* win;

	Stroke currentStroke;
	std::vector<Stroke> strokes;
	std::vector<Stroke> reducedStrokes;
	std::vector<Stroke> polyLines;
	std::vector<Stroke> featureOutlines;
	Stroke buildingOutline;

	Building building;
	BUILDING_ALGORITHM buildingAlgorithm;
	int rotationCount;

	int2 windowSize;
	int verticalDivision;

	MOUSE_ACTION mouseAction;
	bool filled;
	bool showAxis;
	bool mirrorSketch;
	float yaw;
	float pitch;
	float zoom;
	float maxArea;
	int2 dragOrigin;
};

#endif //BUILDINGSKETCH_H