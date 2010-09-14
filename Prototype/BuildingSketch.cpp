#include "BuildingSketch.h"
#include <iostream>
const GLfloat trackballMatrix[4][4] = {
				{1.0, 0.0, 0.0, 0.0},
				{0.0, 1.0, 0.0, 0.0},
				{0.0, 0.0, 1.0, 0.0},
				{0.0, 0.0, 0.0, 1.0}
			};

BuildingSketch::BuildingSketch() : filled(false), yaw(0), pitch(0), zoom(0), extrude(false), showAxis(true) 
{
	MouseAction = NONE;
	windowSize = int2(800, 600);
	verticalDivision = windowSize.x/2;
}

void BuildingSketch::UpdateBuilding()
{
	// TODO: Consider more than the most recent stroke
	building.polys.clear();
	std::vector<int2> outline = polyLines.back().points;

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
	int z_bounds = (extrude) ? 0 : abs(maxCoords.x - minCoords.x);
	building.bounds = int3(abs(maxCoords.x - minCoords.x), abs(maxCoords.y - minCoords.y), z_bounds);
	// Move the building's center to the origin.
	int x_dif = minCoords.x;
	int y_dif = minCoords.y;
	for (unsigned i = 0; i < outline.size(); i++)
	{
		outline[i].x -= x_dif + (building.bounds.x/2);
		outline[i].y -= y_dif + (building.bounds.y/2);
	}


	if (extrude) {		// Algorithm to create a simple extruded building
		std::vector<float3> polyFront;
		polyFront.reserve(outline.size());
		std::vector<float3> polyBack;
		polyBack.reserve(outline.size());
		std::vector<float3> polySide;
		polySide.reserve(4);

		// The depth is 80% of the building base.
		int depth = 0.8*abs(outline[0].x - outline[outline.size()-1].x);

		int2 previous = outline[0];
		for (unsigned i = 0; i < outline.size(); i++)
		{
			int2 current = outline[i];
			polyFront.push_back(float3(current.x, -current.y, -depth/2));
			polyBack.push_back(float3(current.x, -current.y, depth/2));

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
		building.bounds.z = depth;
	}
	else 	// David's algorith (ALGO_MAGICDOTDOTDOT2010 ©) to create mirrored building
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

void BuildingSketch::Process(const Stroke& stroke)
{
	strokes.clear();
	polyLines.clear();
	strokes.push_back(currentStroke); // Record unprocessed stroke
	Stroke reduced = Reduce(stroke, 100.0f);
	polyLines.push_back(reduced);
}

void BuildingSketch::MousePressed(int2 pos)
{
	if (pos.y < verticalDivision) // Mouse pressed on the left side.
	{
		MouseAction = DRAWING;
	}
	else // Mouse pressed on the right side.
	{
		MouseAction = TRACKING;
		// Map the mouse position to a logical sphere location.
		// Keep it in the class variable lastPoint.
		lastPoint = trackBallMapping( pos );
	}
}

void BuildingSketch::MouseReleased(int2 pos)
{	
	switch (MouseAction) 
	{
		case DRAWING:
		{
			// If we just stopped drawing
			MouseAction = NONE;
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
			MouseAction = NONE;
			break;
		}
	}
}

void BuildingSketch::MouseMoved(int2 pos)
{
	float3 curPoint;
	switch (MouseAction) 
	{
		case DRAWING:
		{
			// If we are drawing record movements.
			currentStroke.points.push_back(pos);
			break;
		}
		case TRACKING:
		{
			float m_ROTSCALE = 10.0f;
			float3 direction;
			GLfloat tempMatrix[4][4] = {
				{1.0, 0.0, 0.0, 0.0},
				{0.0, 1.0, 0.0, 0.0},
				{0.0, 0.0, 1.0, 0.0},
				{0.0, 0.0, 0.0, 1.0}
			};

			curPoint = trackBallMapping( pos );  // Map the mouse position to a logical sphere location.
			direction = curPoint - lastPoint;
			float velocity = direction.length();
			if( velocity > 0.0001 ) // If little movement - do nothing.
			{				
				// Rotate about the axis that is perpendicular to the great circle 
				// connecting the mouse movements.
				rotAxis;
				rotAxis = cross( lastPoint, curPoint );
				rot_angle = velocity * m_ROTSCALE;

				// We need to apply the rotation as the last transformation.
				// 1. Get the current matrix and save it.
				// 2. Set the matrix to the identity matrix (clear it).
				// 3. Apply the trackball rotation.
				// 4. Pre-multiply it by the saved matrix.
				/*glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat *) tempMatrix );
				
				glLoadIdentity();
				glRotatef( rot_angle, rotAxis.x, rotAxis.y, rotAxis.z );
				glMultMatrixf( (GLfloat *) trackballMatrix );		
				glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat *) trackballMatrix );	*/	
			}
			// If we want to see it, we need to force the system to redraw the scene.
			//Invalidate( TRUE );

			// Save the location of the current point for the next movement.
			break;
		}
		lastPoint = curPoint;
	}		
}

void BuildingSketch::MouseWheelMoved(int delta)
{
	double ZOOM_SCALE = 5.0;
	if (delta < 0)
	{
		// Zoom in
		zoom = zoom + ZOOM_SCALE;
		zoom = (zoom>30.0) ? 30.0: zoom;
	}
	else if (delta > 0)
	{
		// Zoom out
		zoom = zoom - ZOOM_SCALE;
		zoom = (zoom<-30.0) ? -30.0: zoom;
	} 
}

// Treat the mouse position as the projection of a point on the hemi-sphere
// down to the image plane (along the z-axis), and determine that point on
// the hemi-sphere.
float3 BuildingSketch::trackBallMapping(float2 point)
{
    double3 v;
    double d;
    v.x = (2.0*(point.x - verticalDivision) - windowSize.x) / windowSize.x;
	v.y = ((windowSize.y - verticalDivision) - 2.0*(point.y - verticalDivision)) / (windowSize.y - verticalDivision);
    v.z = 0.0;
    d = v.length();
    d = (d<1.0) ? d : 1.0;
    v.z = sqrt(1.001 - d*d);
    v.normalise(); // Still need to normalize, since we only capped d, not v.
    return v;
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
	glRotatef(pitch, 1, 0, 0);
	glRotatef(yaw, 0, 1, 0);

	//glTranslatef(win->GetWidth()/4, -(win->GetHeight()/2), win->GetWidth()/4);


	glColor3f(0,1, 0);
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
					yaw -= 5; // TODO: Time based
				else if (Event.Key.Code == sf::Key::Right)
					yaw += 5;
				else if (Event.Key.Code == sf::Key::Up)
					pitch += 5; // TODO: Time based
				else if (Event.Key.Code == sf::Key::Down)
					pitch -= 5;
				else if (Event.Key.Code == sf::Key::W)
					filled = !filled;
				else if (Event.Key.Code == sf::Key::A)
					showAxis = !showAxis;
				else if (Event.Key.Code == sf::Key::E) {
					extrude = !extrude;
					UpdateBuilding();
				}
			}

			if (Event.Type == sf::Event::MouseMoved)
				MouseMoved(int2(Event.MouseMove.X, Event.MouseMove.Y));
			if (Event.Type == sf::Event::MouseButtonPressed && Event.MouseButton.Button == sf::Mouse::Left)
				MousePressed(int2(Event.MouseMove.X, Event.MouseMove.Y));
			if (Event.Type == sf::Event::MouseButtonReleased && Event.MouseButton.Button == sf::Mouse::Left)
				MouseReleased(int2(Event.MouseMove.X, Event.MouseMove.Y));
			if (Event.Type == sf::Event::MouseWheelMoved)
				MouseWheelMoved(Event.MouseWheel.Delta);
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