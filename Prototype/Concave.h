#ifndef CONCAVE_H
#define CONCAVE_H

#include <SFML/Graphics.hpp>

#include "Types.h"




// Concave polygon tesselator from:
// http://www.flipcode.com/archives/Polygon_Tessellation_In_OpenGL.shtml
class Tess_Poly {
private:
	GLUtesselator *tobj; // the tessellation object

public:
	Tess_Poly();
	~Tess_Poly();
	//int Init(GLvoid);
	//int Set_Winding_Rule(GLenum winding_rule);
	void Render_Contour(const std::vector<float3>& poly);
	void Begin_Polygon();
	void End_Polygon();
};





#endif // CONCAVE_H
