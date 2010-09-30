#ifndef BUILDINGSKETCH_H
#define BUILDINGSKETCH_H

#include <string>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Concave.h"
#include "Types.h"
#include "Common.h"



class BuildingSketch
{

public:	
	
	BuildingSketch();
	void RenderLoop();		
	
	// Sketch processing/rendering methods
	void ProcessStroke(const Stroke& stroke);
	Stroke Reduce(const Stroke& stroke, float threshold);
	void ResetStrokes();
	void RenderStrokes();
	void DrawStroke(const Stroke& stroke);

	// Building update/rendering methods
	void UpdateBuilding();
	void RenderBuilding();
	void DrawOutline(const Poly poly);

	// User input and event-handling
	void ProcessEvent(sf::Event&);
	void MousePressed(int2 pos);
	void MouseReleased(int2 pos);
	void MouseMoved(int2 pos);
	void MouseWheelMoved(int delta);

	void RandomColor();
	float3 trackBallMapping(float2 point);


private:		// Private methods
	
	// Attempt to change window title - this is currently only possible on a Windows platform
	// Window title will be set to the argument string prepended by defaultAppString
	void ChangeWindowTitle(const std::string&);

	// Update title according to building algorithm and parameters
	void UpdateWindowTitle();

private:		// Private fields
	// Sketch input
	Stroke currentStroke, buildingOutline;
	std::vector<Stroke> strokes, reducedStrokes, polyLines, featureOutlines;
	float maxArea;
		
	// Building algorithm selection, algorithm params and Building output
	enum BUILDING_ALGORITHM { EXTRUDE, ROTATE, MIRROR};
	BUILDING_ALGORITHM buildingAlgorithm;
	int rotationCount;
	bool mirrorSketch;
	Building building;

	// Windowing / OpenGL stuff
	sf::RenderWindow* win;
	Tess_Poly tesselator;
	int2 windowSize;
	int verticalDivision;

	// UI stuff
	enum MOUSE_ACTION { NONE, DRAWING, TRACKING};
	MOUSE_ACTION mouseAction;
	bool filled, showAxis;
	float yaw, pitch, zoom;
	int2 dragOrigin;
	const std::string defaultAppString;

};

#endif //BUILDINGSKETCH_H