#ifndef BUILDINGSKETCH_H
#define BUILDINGSKETCH_H

#include <string>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Concave.h"
#include "Types.h"
#include "Common.h"

class Shader;
struct LineOfSymmetry;
class SymmetryApplication;


class BuildingSketch
{

public:	
	BuildingSketch();
	~BuildingSketch();
	void RenderLoop();		
	
	// Sketch processing/rendering methods
	void ProcessStroke(const Stroke& stroke);
	void ResetStrokes();
	void RenderStrokes();
	void DrawStroke(const Stroke& stroke);

	// Building update/rendering methods
	void UpdateBuilding();
	void RenderBuilding();
	void DrawOutline(const Poly poly);
	void DrawSolid(const Poly poly);

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

	void CleanUpSymmetry();

	// Generates a new sketch based on the strokes returned from symmetry processing
	void GenerateSketch(std::vector<Stroke>&);

private:		// Private fields
	// Sketch input
	Stroke currentStroke, buildingOutline;
	std::vector<Stroke> strokes, reducedStrokes, polyLines, featureOutlines, curves;
	float maxArea;

	// Another data structure for tracking strokes which are drawn (pre-reduction)
	// Used for symmetry calculation
	bool pixels[805][605];
	bool pixelsNear[805][605];

	// Symmetry stuff
	LineOfSymmetry* los;						// last line of symmetry which was calculated
	SymmetryApplication* symm;
	bool losApplicationPending;					// True if a 'good' line of symmetry has been calculated but not acted upon
	
		
	// Building algorithm selection, algorithm params and Building output
	enum BUILDING_ALGORITHM { EXTRUDE, ROTATE, MIRROR, DETECT};
	BUILDING_ALGORITHM buildingAlgorithm;
	int rotationCount;
	bool mirrorSketch;
	Building building;

	BUILDING_ALGORITHM SelectAlgorithm();

	// Windowing / OpenGL stuff
	sf::RenderWindow* win;
	Tess_Poly tesselator;
	int2 windowSize;
	int verticalDivision;

	Shader* buildingShader;
	sf::Image displacementMap2; // TODO: Generate this from features
	sf::Image normalMap;

	// UI stuff
	enum MOUSE_ACTION { NONE, DRAWING, TRACKING};
	MOUSE_ACTION mouseAction;
	bool filled, showAxis;
	float yaw, pitch, zoom;
	int2 dragOrigin;
	const std::string defaultAppString;

};

#endif //BUILDINGSKETCH_H