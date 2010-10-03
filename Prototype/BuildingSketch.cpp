#include <iostream>
#include <SFML/Graphics/GraphicsContext.hpp>
#include "BuildingSketch.h"
#include "BuildingGeneration.h"
#include "SketchPreprocessing.h"
#include "Shader.h"

#ifdef _WIN32
	// Required for ChangeWindowTitle() since SFML doesn't provide a platform-agnostic way
	#include "windows.h"
#endif

using namespace std;




BuildingSketch::BuildingSketch() 
	: filled(true), yaw(45), pitch(25), zoom(0), showAxis(true), defaultAppString("Building Sketch")
{	
	windowSize = int2(800, 600);
	verticalDivision = windowSize.x/2;
	mouseAction = NONE;
	buildingAlgorithm = EXTRUDE;
	mirrorSketch = false;
	rotationCount = 8;
	maxArea = 0;
}

void BuildingSketch::UpdateBuilding()
{
	building.polys.clear();

	building.bounds.x = buildingOutline.bounds.x;
	building.bounds.y = buildingOutline.bounds.y;
	building.bounds.width = buildingOutline.bounds.width;
	building.bounds.height = buildingOutline.bounds.height;
	building.bounds.depth = (buildingAlgorithm == EXTRUDE) ? 0 : buildingOutline.bounds.width;

	std::vector<int2> outline = buildingOutline.points;
	// Move the building's center to the origin.
	int x_dif = building.bounds.x + (building.bounds.width/2);
	int y_dif = building.bounds.y + (building.bounds.height/2);
	for (unsigned i = 0; i < outline.size(); i++)
	{
		outline[i].x -= x_dif;
		outline[i].y -= y_dif;
	}

	// Move the feature outlines accordingly
	std::vector<Stroke> processedFeatureOutlines;
	for (std::vector<Stroke>::iterator s = featureOutlines.begin(); s != featureOutlines.end(); s++)
	{
		Stroke stroke = (*s);
		for (unsigned i = 0; i < stroke.points.size(); i++)
		{
			stroke.points[i].x -= x_dif;
			stroke.points[i].y -= y_dif;
		}
		processedFeatureOutlines.push_back(stroke);
	}


	if (outline.size() == 0)
		return;

	switch (buildingAlgorithm)
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
	reducedStrokes.push_back(reduced);
	
	// Calculate the bounds of each stroke. The modified and bounded strokes
	// will be stored in polyLines and used to generate the 3D building model.
	std::vector<int2> points = reduced.points;
	int2 minCoords = int2(points[0].x,points[0].y);
	int2 maxCoords = int2(points[0].x,points[0].y);
	for (unsigned i = 0; i < points.size(); i++)
	{
		minCoords.x = (minCoords.x < points[i].x) ? minCoords.x : points[i].x;
		minCoords.y = (minCoords.y < points[i].y) ? minCoords.y : points[i].y;
		maxCoords.x = (maxCoords.x > points[i].x) ? maxCoords.x : points[i].x;
		maxCoords.y = (maxCoords.y > points[i].y) ? maxCoords.y : points[i].y;
	}
	// Store the bounds of the stroke
	reduced.bounds.x = minCoords.x;
	reduced.bounds.y = minCoords.y;
	reduced.bounds.width = abs(maxCoords.x - minCoords.x);
	reduced.bounds.height = abs(maxCoords.y - minCoords.y);
	reduced.bounds.depth = 0;

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
	buildingOutline = Stroke();	
	currentStroke = Stroke();
	maxArea = 0;
}


Stroke BuildingSketch::Reduce(const Stroke& stroke, float threshold)
{
	if (stroke.points.size() <= 2)
		return stroke; // Cannot reduce any further
	std::vector<int2> result = DouglasPeuker(stroke.points.begin(), stroke.points.end() - 1, threshold);
	result.push_back(stroke.points.back()); // The recursion will never insert the last point.
	Stroke final;
	final.length = stroke.points.size();
	final.points = result;
	return final;
}

void BuildingSketch::RenderStrokes()
{
	// Draw background
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(10000, 0);
	glVertex2f(10000, 10000);
	glVertex2f(0, 10000);
	glEnd();

	// Draw raw strokes
	glColor3f(0, 0, 0);
	for (std::vector<Stroke>::iterator s = strokes.begin(); s != strokes.end(); s++)
	{
		DrawStroke(*s);
	}
	DrawStroke(currentStroke);

	// Draw processed reduced strokes
	glColor3f(1, 0, 0);
	for (std::vector<Stroke>::iterator s = reducedStrokes.begin(); s != reducedStrokes.end(); s++)
	{
		DrawStroke(*s);
	}
}

void BuildingSketch::DrawStroke(const Stroke& stroke)
{
	glBegin(GL_LINE_STRIP); // Draw raw stroke as line strip
		for (std::vector<int2>::const_iterator v = stroke.points.begin(); v != stroke.points.end(); v++)
			glVertex2iv(v->data);
	glEnd();
}






////////////////////////////////////////////////// Building rendering methods
void BuildingSketch::RenderBuilding()
{
	glColor3f(0.0f, 1.0f, 0.0f);
	for (std::vector<Poly>::iterator p = building.polys.begin(); p != building.polys.end(); p++)
	{
		DrawSolid(*p);
		DrawOutline(*p);
	}

	// Draw axis
	if (showAxis) {
		glBegin(GL_LINES); // Draw line
			glColor3f(0.0f, 1.0f, 0.0f);  // green = x axis
			glVertex3f(0, 0, 0);
			glVertex3f(200, 0, 0);

			glColor3f(1.0f, 1.0f, 1.0f); // white = y axis
			glVertex3f(0, 0, 0);
			glVertex3f(0, 200, 0);

			glColor3f(0.0f, 0.0f, 1.0f); // blue = z axis
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 200);
		glEnd();
	}

	/*glBegin(GL_LINE_STRIP); // Draw raw stroke as line strip
	for (std::vector<float2>::const_iterator v = clipped.begin(); v != clipped.end(); v++)
	glVertex3f(v->x, -v->y, 0);
	glEnd();*/
}

void BuildingSketch::DrawSolid(const Poly poly)
{
	glDisable(GL_CULL_FACE);
	if (filled) {
		glColor3f(0.5f, 0.5f, 0.5f);
		buildingShader->Enable(true);

		glNormal3fv(poly.GetNormal().data);
		glMultiTexCoord3fv(GL_TEXTURE1, poly.GetTangent().data);
		glMultiTexCoord3fv(GL_TEXTURE2, poly.GetBitangent().data);

		// TODO: Support texturing with the tesselator. And make it a one shot process while you're about it
		/*tesselator.Begin_Polygon();
		tesselator.Render_Contour(poly.GetVerts());
		tesselator.End_Polygon();*/

		glBegin(GL_POLYGON); // Draw raw stroke as line strip
		for (std::vector<float3>::const_iterator v = poly.GetVerts().begin(); v != poly.GetVerts().end(); v++)
		{
			float2 uv = poly.GetTexCoords(*v);
			glTexCoord2fv(uv.data);
			glVertex3f(v->x, v->y, v->z);
		}
		glEnd();

		buildingShader->Enable(false);
	}
}

void BuildingSketch::DrawOutline(const Poly poly)
{
	glColor3f(1.0f, 0.0f, 0.0f);
	//RandomColor();
	glBegin(GL_LINE_LOOP); // Draw raw stroke as line loop
		for (std::vector<float3>::const_iterator v = poly.GetVerts().begin(); v != poly.GetVerts().end(); v++)
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

	buildingShader = new Shader("displacement.vert", "displacement.frag"); // TODO: Delete

	while (win->IsOpened())
	{
		sf::Event Event;

		while (win->GetEvent(Event))
			ProcessEvent(Event);


		glClearColor(0, 0, 0, 1);
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

		// Convert newTitle from std::string to wchar_t[]
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
	}

	ChangeWindowTitle(newTitle);
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

	// Escape key pressed
	if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape))
		win->Close();

	if (Event.Type == sf::Event::KeyPressed)
	{
		if (Event.Key.Code == sf::Key::Left)
			yaw = 0;
		else if (Event.Key.Code == sf::Key::Right)
			yaw = 0;
		else if (Event.Key.Code == sf::Key::Up)
			pitch = 0;
		else if (Event.Key.Code == sf::Key::Down)
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
	if (Event.Type == sf::Event::MouseLeft)
		MouseReleased(int2(Event.MouseMove.X, Event.MouseMove.Y));
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
			// If we are drawing record movements.
			currentStroke.points.push_back(pos);
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
