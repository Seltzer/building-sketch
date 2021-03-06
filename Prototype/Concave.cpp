#include <iostream>
#include <SFML/OpenGL.hpp>
#include "Concave.h"

static const Poly* currentPoly = NULL;

void __stdcall vertexCallback(GLvoid *vertex)
{
	double3& v = *(double3*)(vertex);
	//std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	assert(currentPoly);
	float2 texCoord = currentPoly->GetTexCoords(v);
	glTexCoord2fv(texCoord.data);
	glVertex3dv(v.data);
}

/*void __stdcall beginCallback()
{
	glBegin(GL_POLYGON);
}

void __stdcall endCallback()
{
	//glEnd();
	std::cout << std::endl;
}*/

void CALLBACK combineCallback(double coords[3], double *vertex_data[4],
							  float weight[4], double **dataOut)
{
	double *vertex;

	vertex = (double *) malloc(3 * sizeof(double));
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];

	/*for (int i = 3; i < 6; i++)
	{
		vertex[i] = weight[0] * vertex_data[0][i] +
			weight[1] * vertex_data[1][i] +
			weight[2] * vertex_data[2][i] +
			weight[3] * vertex_data[3][i];
	}*/

	*dataOut = vertex;
}

Tess_Poly::Tess_Poly()
{
	// Create a new tessellation object 
	tobj = gluNewTess();

	// Set callback functions
	gluTessCallback(tobj, GLU_TESS_VERTEX, (void (__stdcall *)())&vertexCallback);
	gluTessCallback(tobj, GLU_TESS_BEGIN, (void (__stdcall *)())&glBegin);
	gluTessCallback(tobj, GLU_TESS_END, (void (__stdcall *)())&glEnd);
	gluTessCallback(tobj, GLU_TESS_COMBINE, (void (__stdcall *)())&combineCallback);

	gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO); 
}

Tess_Poly::~Tess_Poly()
{
	gluDeleteTess(tobj);
}

void Tess_Poly::Render_Contour(const Poly& poly)
{
	currentPoly = &poly;
	static std::vector<double3> newPoly;
	newPoly.resize(poly.GetVerts().size());
	for (unsigned i = 0; i < newPoly.size(); i++)
		newPoly[i] = double3(poly[i].x, poly[i].y, poly[i].z);

	gluTessBeginContour(tobj);
	for (unsigned x = 0; x < newPoly.size(); x++) //loop through the vertices
	{
		gluTessVertex(tobj, newPoly[x].data, newPoly[x].data); //store the vertex
	}
	gluTessEndContour(tobj);
}

void Tess_Poly::Begin_Polygon(GLvoid)
{
	gluTessBeginPolygon(tobj, NULL);
}

void Tess_Poly::End_Polygon(GLvoid)
{
	gluTessEndPolygon(tobj);
}
