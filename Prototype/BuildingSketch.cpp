#include <iostream>
#include <SFML/Graphics/GraphicsContext.hpp>
#include "BuildingSketch.h"
#include "BuildingGeneration.h"
#include "SketchPreprocessing.h"
#include "SymmetryDetection.h"
#include "DisplacementMapping.h"
#include "Shader.h"
#include "HeightmapProcessing.h"

#ifdef _WIN32
	// Required for ChangeWindowTitle() since SFML doesn't provide a platform-agnostic way
	#include "windows.h"
#endif

using namespace std;




BuildingSketch::BuildingSketch() 
	: maxArea(0), los(NULL), symm(NULL), buildingAlgorithm(EXTRUDE), losApplicationPending(false), rotationCount(8), mirrorSketch(false), filled(true), yaw(45), pitch(25), zoom(0), 
			windowSize(800, 600), mouseAction(NONE), showAxis(true), defaultAppString("Building Sketch")
{	
	verticalDivision = windowSize.x/2;

	ResetStrokes();
}

BuildingSketch::~BuildingSketch()
{
	CleanUpSymmetry();
}

bool IsCurved(const Stroke stroke)
{
	const std::vector<int2>& points = stroke.points;
	std::vector<float> angles;
	angles.reserve(points.size());
	float avgAngle = 0.0f;
	float totalAngle = 0.0f;
	float maxAngle = 0.0f;

	if (points.size() < 3)
		return false;

	for (unsigned i = 1; i < points.size()-1; i++)
	{
		int2 prev = points[i-1];
		int2 cur = points[i];
		int2 next = points[i+1];
		float2 seg1 = cur - prev;
		float2 seg2 = next - cur;
		float dist = (seg1.length() + seg2.length()) / 2.0f;
		float angle = atan2f(seg2.y, seg2.x) - atan2f(seg1.y, seg1.x);
		angles.push_back(angle / dist);
		avgAngle += angle / dist;
		totalAngle += angle;
		maxAngle = std::max(maxAngle, abs(angle));
	}

	float variance = 0.0f;
	for (unsigned i = 0; i < angles.size(); i++)
	{
		float diff = angles[i] - avgAngle;
		variance += diff * diff;
	}
	variance /= angles.size();

	float stddev = sqrt(variance);

	//std::cout << "angle: " << totalAngle << " avgAngle: " << avgAngle << " stddev: " << stddev << " maxangle: " << maxAngle <<  std::endl;

	if (maxAngle > 3.14169f/3)
		return false;

	// If angle is more than a few degrees, fairly consistently one way (some num standard deviations from 0)
	// and angle is less than 180 (not a proper feature or outline) we have our curve.
	return (abs(totalAngle) > 20*3.14159/180 && abs(totalAngle) < 3.14159) && abs(avgAngle) > 1.3 * stddev;
}

BuildingSketch::BUILDING_ALGORITHM BuildingSketch::SelectAlgorithm()
{
	// Determine if round. While ideally we would determine if parts of the building are round, for now we will just do the whole thing
	if (curves.size() > 0) // If there is a curve
		return ROTATE; // Assume whole building is round

	if (buildingOutline.bounds.height > 1.5f * buildingOutline.bounds.width)
		return MIRROR;
	else
		return EXTRUDE;
}

void BuildingSketch::UpdateBuilding()
{
	building.polys.clear();

	vector<Stroke> features = featureOutlines;

	BUILDING_ALGORITHM algo = buildingAlgorithm;

	if (buildingAlgorithm == DETECT)
		algo = SelectAlgorithm();

	// Building bounds are the same as sketch bounds, possibly apart from the depth
	building.bounds = buildingOutline.bounds;
	building.bounds.depth = (algo == EXTRUDE) ? 0 : buildingOutline.bounds.width;

	vector<int2> outline = buildingOutline.points;
	if (outline.size() == 0)
		return;
	// Move the building's center to the origin.
	int x_dif = building.bounds.x + (building.bounds.width/2);
	int y_dif = building.bounds.y + (building.bounds.height/2);
	for (unsigned i = 0; i < outline.size(); i++)
	{
		outline[i].x -= x_dif;
		outline[i].y -= y_dif;
	}

	// Move the feature outlines accordingly
	vector<Stroke> processedFeatureOutlines;
	for (vector<Stroke>::iterator s = features.begin(); s != features.end(); s++)
	{
		Stroke stroke = (*s);
		for (unsigned i = 0; i < stroke.points.size(); i++)
		{
			stroke.points[i].x -= x_dif;
			stroke.points[i].y -= y_dif;
		}
		stroke.bounds.x -= x_dif;
		stroke.bounds.y -= y_dif;
		stroke.points.push_back(stroke.points[0]);
		processedFeatureOutlines.push_back(stroke);
	}

	// Move the feature outlines to a 0,0 co-ordinate & generate a displacement map
	std::vector<Stroke> displacementMapStrokes;
	for (vector<Stroke>::iterator s = processedFeatureOutlines.begin(); s != processedFeatureOutlines.end(); s++)
	{
		Stroke stroke = (*s);
		for (unsigned i = 0; i < stroke.points.size(); i++)
		{
			stroke.points[i].x += building.bounds.width/2;
			stroke.points[i].y += building.bounds.height/2;
		}
		stroke.bounds.x += building.bounds.width/2;
		stroke.bounds.y += building.bounds.height/2;
		displacementMapStrokes.push_back(stroke);
	}

	// Generate the displacement map
	generateDisplacementMap(building.bounds, displacementMapStrokes);
	normalMap = heightToNormal(displacementMap);

	switch (algo)
	{
		case EXTRUDE:	// Algorithm to create an extruded building
		{	
			ExtrudeSketch(building, outline, processedFeatureOutlines);
			break;
		}
		case ROTATE:	// Algorithm to create an rotated building
		{
			RotateSketch(building, outline, rotationCount, mirrorSketch);
			break;
		}
		case MIRROR: 	// David's algorith (ALGO_MAGICDOTDOTDOT2010 ©) to create mirrored building
		{
			MirrorSketch(building, outline);
		}
	}
}

void BuildingSketch::ProcessStroke(const Stroke& stroke)
{
	strokes.push_back(currentStroke); // Record unprocessed stroke
	Stroke reduced = Reduce(stroke, 30.0f);

	//std::cout << IsCurved(reduced) << std::endl;
		
	// Calculate the bounds of each stroke. The modified and bounded strokes
	// will be stored in polyLines and used to generate the 3D building model.
	reduced.CalculateBounds();

	if (IsCurved(reduced)) {
		curves.push_back(reduced);
		return; // Do not consider as anything else
	}

	// The stroke that encompasses the largest area is the outline
	// All other strokes should be treated as features.
	float strokeArea = reduced.bounds.height * reduced.bounds.width;
	if (strokeArea > maxArea) {
		if (maxArea > 0) featureOutlines.push_back(buildingOutline);
		buildingOutline = reduced;
		maxArea = strokeArea;
	} else {
		featureOutlines.push_back(reduced);
	}
	
	reducedStrokes.push_back(reduced);
}

/* 
 * Reset all the stroke vectors.
 */
void BuildingSketch::ResetStrokes() 
{	
	strokes.clear();
	reducedStrokes.clear();		
	polyLines.clear();
	featureOutlines.clear();
	curves.clear();

	buildingOutline = Stroke();	
	currentStroke = Stroke();
	
	CleanUpSymmetry();
	maxArea = 0;

	// Clear pixels and pixelsNear
	StrokesToMatrices<805,605>(pixels, pixelsNear, 0, vector<Stroke>());
}

void BuildingSketch::RenderStrokes()
{
	// Draw background
	glColor3f((248.0/255.0), (248/255.0), (245/255.0));
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(10000, 0);
	glVertex2f(10000, 10000);
	glVertex2f(0, 10000);
	glEnd();

	// Draw raw strokes
	glColor3f(0, 0, 0);
	for (vector<Stroke>::iterator s = strokes.begin(); s != strokes.end(); s++)
	{
		DrawStroke(*s);
	}
	DrawStroke(currentStroke);

	// Draw processed reduced strokes
	glColor3f(1, 0, 0);
	for (vector<Stroke>::iterator s = reducedStrokes.begin(); s != reducedStrokes.end(); s++)
	{
		DrawStroke(*s);
	}

	// Draw curves
	glColor3f(1, 1, 0);
	for (vector<Stroke>::iterator s = curves.begin(); s != curves.end(); s++)
	{
		DrawStroke(*s);
	}

	// Draw line of symmetry in green
	if (los != NULL)
	{
		int2 losEnd = los->pointOnLine + 500 * los->direction;

		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
			glVertex2f(los->pointOnLine.x, los->pointOnLine.y);
			glVertex2f(losEnd.x, losEnd.y);
		glEnd();
	}
}

void BuildingSketch::DrawStroke(const Stroke& stroke)
{
	glBegin(GL_LINE_STRIP); // Draw raw stroke as line strip
		for (vector<int2>::const_iterator v = stroke.points.begin(); v != stroke.points.end(); v++)
			glVertex2iv(v->data);
	glEnd();
}

////////////////////////////////////////////////// Building rendering methods
void BuildingSketch::RenderBuilding()
{
	glColor3f(0.0f, 1.0f, 0.0f);
	for (vector<Poly>::iterator p = building.polys.begin(); p != building.polys.end(); p++)
	{
		DrawSolid(*p);
		DrawOutline(*p);
	}

	// Draw axis
	if (showAxis) {
		glBegin(GL_LINES); // Draw line
			glColor3f(0.0f, 0.0f, 0.0f);  // black = x axis
			glVertex3f(0, 0, 0);
			glVertex3f(200, 0, 0);

			glColor3f(1.0f, 1.0f, 1.0f); // white = y axis
			glVertex3f(0, 0, 0);
			glVertex3f(0, 200, 0);

			glColor3f(1.0f, 0.0f, 0.0f); // red = z axis
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 200);
		glEnd();
	}

	/*glBegin(GL_LINE_STRIP); // Draw raw stroke as line strip
	for (vector<float2>::const_iterator v = clipped.begin(); v != clipped.end(); v++)
	glVertex3f(v->x, -v->y, 0);
	glEnd();*/
}

void BuildingSketch::DrawSolid(const Poly poly)
{
	glDisable(GL_CULL_FACE);
	if (filled) {
		glColor3f((249.0/255.0), (249.0/255.0), (240.0/255.0));
		buildingShader->Enable(true);
		displacementMap.Bind();

		buildingShader->BindTexture(displacementMap, "reliefmap", 0);
		buildingShader->BindTexture(normalMap, "normalmap", 1);


		glNormal3fv(poly.GetNormal().data);
		glMultiTexCoord3fv(GL_TEXTURE1, poly.GetTangent().data);
		glMultiTexCoord3fv(GL_TEXTURE2, poly.GetBitangent().data);

		// TODO: Make tesselating a one shot process
		tesselator.Begin_Polygon();
		tesselator.Render_Contour(poly);
		tesselator.End_Polygon();

		/*glBegin(GL_POLYGON); // Draw raw stroke as line strip
		for (vector<float3>::const_iterator v = poly.GetVerts().begin(); v != poly.GetVerts().end(); v++)
		{
			float2 uv = poly.GetTexCoords(*v);
			glTexCoord2fv(uv.data);
			glVertex3f(v->x, v->y, v->z);
		}
		glEnd();*/

		buildingShader->Enable(false);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
	}
}

void BuildingSketch::DrawOutline(const Poly poly)
{
	glColor3f(0.65f, 0.0f, 0.0f);
	//RandomColor();
	glBegin(GL_LINE_LOOP); // Draw raw stroke as line loop
		for (vector<float3>::const_iterator v = poly.GetVerts().begin(); v != poly.GetVerts().end(); v++)
		{
			//glColor3f(1.0f, (v - poly.begin()) / (poly.size() - 1), 0.0f);
			glVertex3f(v->x, v->y, v->z);
		}
	glEnd();
}


void BuildingSketch::RenderLoop()
{
	win = new sf::RenderWindow(sf::VideoMode(windowSize.x, windowSize.y, 32), defaultAppString, sf::Style::Resize|sf::Style::Close);
	win->SetActive(true);
	win->PreserveOpenGLStates(true);
	ChangeWindowTitle(defaultAppString + " - Extrusion Mode");

	buildingShader = new Shader("displacement.vert", "displacement.frag"); // TODO: Memory leak
	//displacementMap2.LoadFromFile("collage_height.jpg");
	displacementMap.Create(32, 32); // Blank map to begin with
	normalMap = heightToNormal(displacementMap);

	while (win->IsOpened())
	{
		sf::Event Event;

		while (win->GetEvent(Event))
			ProcessEvent(Event);


		glClearColor((51.0/255.0), (51.0/255.0), (102.0/255.0), 1); //left panel background color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glViewport(0, 0, verticalDivision, win->GetHeight());		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, verticalDivision, win->GetHeight(), 0, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		RenderStrokes();


		glViewport(verticalDivision, 0, (win->GetWidth() - verticalDivision), win->GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45 + zoom, (float(win->GetWidth())  - verticalDivision) / win->GetHeight(), 10.0f, 10000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();			
		glEnable(GL_DEPTH_TEST);
		glTranslatef(0, 0, -1000); // Move it to an appropriate position to view.
		glRotatef(pitch, 1, 0, 0);
		glRotatef(yaw, 0, 1, 0);

		RenderBuilding();
		glDisable(GL_DEPTH_TEST);

		win->Display();

		sf::Sleep(1.0f / 60);
	}
}

void BuildingSketch::RandomColor()
{
	glColor3f(randFloat(), randFloat(), randFloat());
}


void BuildingSketch::ChangeWindowTitle(const string& newTitle)
{
	#ifdef _WIN32
		// Hack to get HWND for SFML win
		HWND windowHandle = (HWND) win->GetWindowHandle();
		if (!windowHandle)
			return;

		// Convert newTitle from string to wchar_t[]
		// TODO: This conversion will not work for systems without Unicode support
        wchar_t convertedTitle[256];
        int NbChars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, newTitle.c_str(), static_cast<int>(newTitle.size()), convertedTitle, sizeof(convertedTitle) / sizeof(*convertedTitle));
        convertedTitle[NbChars] = L'\0';

		SetWindowText(windowHandle, convertedTitle);
	#endif
}

void BuildingSketch::UpdateWindowTitle()
{
	string newTitle(defaultAppString);

	switch(buildingAlgorithm)
	{
		case EXTRUDE:
			newTitle.append(" - Extrusion Mode");
			break;
		case MIRROR:
			newTitle.append(" - Mirror Mode");
			break;
		case ROTATE:
			newTitle.append(" - Rotation Mode with arity of " + ConvertToString<int>(rotationCount));
			break;
		case DETECT:
			newTitle.append(" - Detect Mode");
			break;
	}

	ChangeWindowTitle(newTitle);
}

void BuildingSketch::CleanUpSymmetry()
{
	losApplicationPending = false;

	if (symm != NULL)
	{
		delete symm;
		symm = NULL;
	}

	if (los != NULL)
	{
		delete los;
		los = NULL;
	}
}

void BuildingSketch::GenerateSketch(vector<Stroke>& strokes)
{
	ResetStrokes();
	StrokesToMatrices<805, 605>(pixels, pixelsNear, 1, strokes);

	for (vector<Stroke>::iterator it = strokes.begin(); it < strokes.end(); it++)
	{
		// We don't care about dots
		if ((*it).points.size() >=2) 
			ProcessStroke(*it); // Process the stroke
	}
	
	UpdateBuilding();
	CleanUpSymmetry();
}





//////////////////////////////////////////////////// Events
void BuildingSketch::ProcessEvent(sf::Event& Event)
{
	// window resized
	if (Event.Type == sf::Event::Resized)
	{
		int width = Event.Size.Width;
		int height = Event.Size.Height;
		win->SetView(sf::View(sf::FloatRect(0, 0, (float) width, (float) height))); // Not needed?
		windowSize = int2(width, height);
		verticalDivision = width/2;
	}

	// Window closed
	if (Event.Type == sf::Event::Closed)
		win->Close();

	if (Event.Type == sf::Event::KeyPressed)
	{
		if (Event.Key.Code == sf::Key::Escape)
		{
			if (losApplicationPending)
				CleanUpSymmetry();
			else
				win->Close();
		}
		else if (Event.Key.Code == sf::Key::Left || Event.Key.Code == sf::Key::Right)
			yaw = 0;
		else if (Event.Key.Code == sf::Key::Up || Event.Key.Code == sf::Key::Down)
			pitch = 0;
		// Toggle wireframe fill on and off.
		else if (Event.Key.Code == sf::Key::W)
			filled = !filled;
		// Toggle axis on and off.
		else if (Event.Key.Code == sf::Key::A)
			showAxis = !showAxis;
		// Toggle mirror sketch in rotated algorithm.
		else if (Event.Key.Code == sf::Key::M)
		{				
			mirrorSketch = !mirrorSketch;
			UpdateBuilding();
		}
		// Calculate a line of symmetry
		else if (Event.Key.Code == sf::Key::S)
		{
			CleanUpSymmetry();

			symm = new SymmetryApplication(buildingOutline);
			
			symm->InterpolateSketch(strokes);
			symm->CalculateSymmetry();
						
			if (symm->LOSIsValid())
			{
				los = new LineOfSymmetry(symm->GetLOS());
				losApplicationPending = true;
			}
		}
		else if (Event.Key.Code == sf::Key::L)
		{
			if (losApplicationPending)
			{
				vector<Stroke> generated = symm->ApplyLOS(pixels, pixelsNear, strokes, true);
				GenerateSketch(generated);
			}
		}
		else if (Event.Key.Code == sf::Key::R)
		{
			if (losApplicationPending)
			{
				vector<Stroke> generated = symm->ApplyLOS(pixels, pixelsNear, strokes, false);
				GenerateSketch(generated);
			}
		}
		// Increase the rations count for the rotation algorith.
		else if (Event.Key.Code == sf::Key::Comma)
		{
			rotationCount--;
			rotationCount = (rotationCount<1) ? 1 : rotationCount;
			UpdateWindowTitle();
			UpdateBuilding();
		}
		// Decrease the rations count for the rotation algorith.
		else if (Event.Key.Code == sf::Key::Period)
		{
			++rotationCount;
			UpdateWindowTitle();
			UpdateBuilding();
		}
		// Toggle differnt building render algorithms
		else if (Event.Key.Code == sf::Key::E) {	
			switch (buildingAlgorithm) 
			{
				case EXTRUDE:
				{
					buildingAlgorithm = ROTATE;
					break;
				}
				case ROTATE:
				{
					buildingAlgorithm = MIRROR;
					break;
				}
				case MIRROR:
				{
					buildingAlgorithm = DETECT;
					break;
				}
				case DETECT:
				{
					buildingAlgorithm = EXTRUDE;
					break;
				}
			}
			UpdateWindowTitle();
			UpdateBuilding();
		}
		else if (Event.Key.Code == sf::Key::Space)
		{
			ResetStrokes();
			UpdateBuilding();
		}
	}

	if (Event.Type == sf::Event::MouseMoved)
		MouseMoved(int2(Event.MouseMove.X, Event.MouseMove.Y));
	if (Event.Type == sf::Event::MouseButtonPressed && Event.MouseButton.Button == sf::Mouse::Left)
		MousePressed(int2(Event.MouseButton.X, Event.MouseButton.Y));
	if (Event.Type == sf::Event::MouseButtonReleased && Event.MouseButton.Button == sf::Mouse::Left)
		MouseReleased(int2(Event.MouseMove.X, Event.MouseMove.Y));
	if (Event.Type == sf::Event::MouseWheelMoved)
		MouseWheelMoved(Event.MouseWheel.Delta);
}

void BuildingSketch::MousePressed(int2 pos)
{
	if (pos.x < verticalDivision) // Mouse pressed on the left panel.
	{
		mouseAction = DRAWING;
	}
	else // Mouse pressed on the right panel.
	{
		mouseAction = TRACKING;
		dragOrigin = pos;
	}
}

void BuildingSketch::MouseReleased(int2 pos)
{	
	switch (mouseAction) 
	{
		case DRAWING:
		{
			if (currentStroke.points.size() >=2) // We don't care about dots
			{
				ProcessStroke(currentStroke); // Process the stroke
				UpdateBuilding();
			}
			currentStroke = Stroke(); // Reset stroke for next drawing
			break;
		}
		case TRACKING:
		{
			break;
		}
	}
	mouseAction = NONE;
}

void BuildingSketch::MouseMoved(int2 pos)
{
	switch (mouseAction) 
	{
		case DRAWING:
		{
			// Don't go beyone vertical division
			if (pos.x > verticalDivision)
				pos.x = verticalDivision;

			// If we are drawing record movements.
			currentStroke.points.push_back(pos);
			pixels[pos.x][pos.y] = true;
			
			for (int x = -1; x <= 1; x++)
			{
				for (int y = -1; y <= 1; y++)
				{
					pixelsNear[pos.x + x][pos.y + y] = true;
				}
			}


			break;
		}
		case TRACKING:
		{
			float TRACK_SCALE = 0.4f;
			pitch += (pos.y - dragOrigin.y)*TRACK_SCALE;
			// Gimbal lock fix
			// http://en.wikipedia.org/wiki/Gimbal_lock
			if (pitch > 360)
				pitch = 0;
			if (pitch >= 90 && pitch < 270) {
				yaw -= (pos.x - dragOrigin.x)*TRACK_SCALE;
			} else {
				yaw += (pos.x - dragOrigin.x)*TRACK_SCALE;
			}
			dragOrigin = pos;			
		}
	}		
}

void BuildingSketch::MouseWheelMoved(int delta)
{
	float ZOOM_SCALE = 5.0;
	if (delta < 0)
	{
		// Zoom in
		zoom = zoom + ZOOM_SCALE;
		zoom = (zoom>30.0f) ? 30.0f: zoom;
	}
	else if (delta > 0)
	{
		// Zoom out
		zoom = zoom - ZOOM_SCALE;
		zoom = (zoom<-30.0f) ? -30.0f: zoom;
	} 
}
