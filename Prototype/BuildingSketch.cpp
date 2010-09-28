#include "BuildingSketch.h"
#include <iostream>

BuildingSketch::BuildingSketch() : filled(true), yaw(45), pitch(25), zoom(0), showAxis(true) 
{
	mouseAction = NONE;
	buildingAlgorithm = ROTATE;
	mirrorSketch = false;
	rotationCount = 8;
	windowSize = int2(800, 600);
	verticalDivision = windowSize.x/2;
}

void BuildingSketch::UpdateBuilding()
{
	// TODO: Consider more than the most recent stroke
	building.polys.clear();
	// The longest stroke is the outline
	std::vector<int2> outline = (*polyLines.begin()).points;
	int maxLength = (*polyLines.begin()).length;
	for (std::vector<Stroke>::iterator s = polyLines.begin() + 1; s != polyLines.end(); s++)
	{
		if ((*s).length > maxLength) {
			outline = (*s).points;
		}
	}

	// Calculate the bounds of the building
	int2 minCoords = int2(outline[0].x,outline[0].y);
	int2 maxCoords = int2(outline[0].x,outline[0].y);
	for (unsigned i = 0; i < outline.size(); i++)
	{
		minCoords.x = (minCoords.x < outline[i].x) ? minCoords.x : outline[i].x;
		minCoords.y = (minCoords.y < outline[i].y) ? minCoords.y : outline[i].y;
		maxCoords.x = (maxCoords.x > outline[i].x) ? maxCoords.x : outline[i].x;
		maxCoords.y = (maxCoords.y > outline[i].y) ? maxCoords.y : outline[i].y;
	}
	building.bounds.width = abs(maxCoords.x - minCoords.x);
	building.bounds.height = abs(maxCoords.y - minCoords.y);
	building.bounds.depth = (buildingAlgorithm == EXTRUDE) ? 0 : abs(maxCoords.x - minCoords.x);
	// Move the building's center to the origin.
	int x_dif = minCoords.x;
	int y_dif = minCoords.y;
	for (unsigned i = 0; i < outline.size(); i++)
	{
		outline[i].x -= x_dif + (building.bounds.width/2);
		outline[i].y -= y_dif + (building.bounds.height/2);
	}

	switch (buildingAlgorithm)
	{
		case EXTRUDE:	// Algorithm to create an extruded building
		{		
			std::vector<float3> polyFront;
			polyFront.reserve(outline.size());
			std::vector<float3> polyBack;
			polyBack.reserve(outline.size());
			std::vector<float3> polySide;
			polySide.reserve(4);

			// The depth is 80% of the building base.
			float depth = 0.8f*abs(building.bounds.width);

			float2 previous = outline[0];
			for (unsigned i = 0; i < outline.size(); i++)
			{
				float2 current = outline[i];
				polyFront.push_back(float3(current.x, -current.y, -depth/2.0f));
				polyBack.push_back(float3(current.x, -current.y, depth/2.0f));

				if (i > 0) {
					polySide.clear();
					polySide.push_back(float3(current.x, -current.y, -depth/2));
					polySide.push_back(float3(current.x, -current.y, depth/2));
					polySide.push_back(float3(previous.x, -previous.y, depth/2));
					polySide.push_back(float3(previous.x, -previous.y, -depth/2));
					building.polys.push_back(polySide);
				}
				previous = current;
			}
			building.polys.push_back(polyFront);
			building.polys.push_back(polyBack);
			building.bounds.depth = (int)depth;
			break;
		}
		case ROTATE:	// Algorithm to create an rotated building
		{
			float rotAngle = (float)(D_PI/(rotationCount+1)); // rotate by atleast 90 degrees
			std::vector<float3> polySide;
			polySide.reserve(4);

			// Convert old 2D outline to a 3D outline.			
			std::vector<float3> oldOutline;
			oldOutline.reserve(outline.size());
			for (unsigned i = 0; i < outline.size(); i++)
			{
				float3 point = float3((float)outline[i].x, (float)outline[i].y, 0);
				oldOutline.push_back(point);
			}

			// Mirror the sketch so everything lines up on a central axis
			if (mirrorSketch) {
				for (unsigned i = 0; i < oldOutline.size()/2; i++)
				{
					oldOutline[oldOutline.size()-1-i].x = -oldOutline[i].x;
					oldOutline[oldOutline.size()-1-i].y = oldOutline[i].y;
				}
			}

			for (int j = 0; j < (rotationCount+1)*2; j++)
			{	
				// rotate the old outline by rotAngle to get the new outline.
				std::vector<float3> newOutline = oldOutline;
				for (unsigned i = 0; i < newOutline.size(); i++)
				{
					float temp_x = newOutline[i].z*sin(rotAngle) + newOutline[i].x*cos(rotAngle);
					float temp_z = newOutline[i].z*cos(rotAngle) - newOutline[i].x*sin(rotAngle);
					newOutline[i].x = temp_x;
					newOutline[i].z = temp_z;
				}

				// Build each side polygon
				float3 prevPoint_oldOutline = oldOutline[0];
				float3 prevPoint_newOutline = newOutline[0];
				for (unsigned i = 1; i < newOutline.size(); i++)
				{
					float3 currentPoint_newOutline = newOutline[i];
					float3 currentPoint_oldOutline = oldOutline[i];
					polySide.clear();
					polySide.push_back(float3(currentPoint_newOutline.x, -currentPoint_newOutline.y, currentPoint_newOutline.z));
					polySide.push_back(float3(currentPoint_oldOutline.x, -currentPoint_oldOutline.y, currentPoint_oldOutline.z));					
					polySide.push_back(float3(prevPoint_oldOutline.x, -prevPoint_oldOutline.y, prevPoint_oldOutline.z));
					polySide.push_back(float3(prevPoint_newOutline.x, -prevPoint_newOutline.y, prevPoint_newOutline.z));
					building.polys.push_back(polySide);

					prevPoint_oldOutline = currentPoint_oldOutline;
					prevPoint_newOutline = currentPoint_newOutline;
				}
				oldOutline = newOutline;
			}			
			break;
		}
		case MIRROR: 	// David's algorith (ALGO_MAGICDOTDOTDOT2010 ©) to create mirrored building
		{
			// Magic... TODO: Proper comments
			int2 previous = outline[0];
			for (unsigned i = 1; i < outline.size(); i++)
			{
				int2 current = outline[i];

				int yMax = std::max(previous.y, current.y);
				int yMin = std::min(previous.y, current.y);

				//assert(yMin != yMax); // TODO: Handle special case

				std::vector< std::vector<float2> > polys2d = Clip(outline, yMin, yMax);

				for (unsigned p = 0; p < polys2d.size(); p++)
				{
					const std::vector<float2>& poly2d = polys2d[p];

					std::vector<float3> polyFront;
					polyFront.reserve(poly2d.size());
					std::vector<float3> polySide;
					polySide.reserve(poly2d.size());
					for (unsigned j = 0; j < poly2d.size(); j++)
					{
						float2 p2d = poly2d[j];
						// Interpolate based on y coordinate
						float z = previous.x + float(p2d.y - previous.y) / (current.y - previous.y) * (current.x - previous.x);
						polyFront.push_back(float3(p2d.x, -p2d.y, z));
						polySide.push_back(float3(z, -p2d.y, p2d.x));
					}
					building.polys.push_back(polyFront);
					building.polys.push_back(polySide);
				}
				previous = current;
			}
		}
	}
}

void BuildingSketch::Process(const Stroke& stroke)
{
	strokes.push_back(currentStroke); // Record unprocessed stroke
	Stroke reduced = Reduce(stroke, 100.0f);
	polyLines.push_back(reduced);
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
				Process(currentStroke); // Process
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

void BuildingSketch::RenderLines()
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

	// Draw processed strokes
	glColor3f(1, 0, 0);
	for (std::vector<Stroke>::iterator s = polyLines.begin(); s != polyLines.end(); s++)
	{
		DrawStroke(*s);
	}
}

void BuildingSketch::RenderBuilding()
{
	glColor3f(0.0f, 1.0f, 0.0f);
	for (std::vector<Poly>::iterator p = building.polys.begin(); p != building.polys.end(); p++)
	{
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

void BuildingSketch::RenderLoop()
{
	win = new sf::RenderWindow(sf::VideoMode(windowSize.x, windowSize.y, 32), "Building Sketch");
	win->SetActive(true);
	win->PreserveOpenGLStates(true);

	while (win->IsOpened())
	{
		sf::Event Event;
		while (win->GetEvent(Event))
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
					yaw = 0; // TODO: Time based
				else if (Event.Key.Code == sf::Key::Right)
					yaw = 0;
				else if (Event.Key.Code == sf::Key::Up)
					pitch = 0; // TODO: Time based
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
					UpdateBuilding();
				}
				// Decrease the rations count for the rotation algorith.
				else if (Event.Key.Code == sf::Key::Period)
				{
					rotationCount++;
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
					UpdateBuilding();
				}
				else if (Event.Key.Code == sf::Key::Space)
				{
					strokes.clear();
					polyLines.clear();
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

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glViewport(0, 0, verticalDivision, win->GetHeight());		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, verticalDivision, win->GetHeight(), 0, -100, 100);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		RenderLines();


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

std::vector<int2> BuildingSketch::DouglasPeuker(std::vector<int2>::const_iterator start, std::vector<int2>::const_iterator end, float threshold)
{
	std::vector<int2> result;
	if (end - start <= 1) { // Can't subdivide any more
		result.push_back(*start);
		return result;
	}

	// Find line parameters
	int2 lineStart = *start;
	int2 lineEnd = *end;
	int2 diff = lineEnd - lineStart;
	float2 dir = normal(float2(float(diff.x), float(diff.y)));
	float2 ortho(-dir.y, dir.x);

	// Find point with greatest error
	std::vector<int2>::const_iterator mid = start + 1;
	float maxError = 0;
	for (std::vector<int2>::const_iterator i = start + 1; i != end; i++)
	{
		int2 rel = *i - lineStart;
		float dist = dot(ortho, float2(float(rel.x), float(rel.y)));
		float error = dist * dist; // Squared distance
		if (error > maxError)
		{
			maxError = error;
			mid = i;
		}
	}

	if (maxError >= threshold)
	{
		result = DouglasPeuker(start, mid, threshold);
		std::vector<int2> rhs = DouglasPeuker(mid, end, threshold);
		result.insert(result.end(), rhs.begin(), rhs.end());
		return result;
	}

	// Insufficient error, just return single line segment
	result.push_back(*start);
	return result;
}

BuildingSketch::Stroke BuildingSketch::Reduce(const Stroke& stroke, float threshold)
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

void BuildingSketch::DrawStroke(const Stroke& stroke)
{
	glBegin(GL_LINE_STRIP); // Draw raw stroke as line strip
		for (std::vector<int2>::const_iterator v = stroke.points.begin(); v != stroke.points.end(); v++)
			glVertex2iv(v->data);
	glEnd();
}

float BuildingSketch::randFloat()
{
	return rand() / float(RAND_MAX);
}

void BuildingSketch::RandomColor()
{
	glColor3f(randFloat(), randFloat(), randFloat());
}

void BuildingSketch::DrawOutline(const Poly poly)
{
	glColor3f(0.5f, 0.5f, 0.5f);
	/*RandomColor();
	glBegin(GL_POLYGON); // Draw raw stroke as line strip
		for (std::vector<float3>::const_iterator v = poly.begin(); v != poly.end(); v++)
			glVertex3f(v->x, -v->y, v->z);
	glEnd();*/
	glDisable(GL_CULL_FACE);
	if (filled) {
		tesselator.Begin_Polygon();
		tesselator.Render_Contour(poly);
		tesselator.End_Polygon();
	}

	glColor3f(1.0f, 0.0f, 0.0f);
	//RandomColor();
	glBegin(GL_LINE_LOOP); // Draw raw stroke as line loop
		for (std::vector<float3>::const_iterator v = poly.begin(); v != poly.end(); v++)
		{
			//glColor3f(1.0f, (v - poly.begin()) / (poly.size() - 1), 0.0f);
			glVertex3f(v->x, v->y, v->z);
			
		}
	glEnd();
}