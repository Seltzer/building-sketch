#include "BuildingGeneration.h"
#include "Clip.h"

using namespace std;


void ExtrudeSketch(Building& building, vector<int2>& outline, vector<Stroke>& processedFeatureOutlines)
{
	std::vector<float3> featurePolyFront;
	std::vector<float3> featurePolyBack;
	std::vector<float3> featurePolySide;
	std::vector<float3> polyFront;
	std::vector<float3> polyBack;
	std::vector<float3> polySide;

	polyFront.reserve(outline.size());
	polyBack.reserve(outline.size());
	polySide.reserve(4);

	float tangScale = float(building.bounds.height) / building.bounds.width; // Hack to fix distortion

	// The depth is 80% of the building width.
	float depth = 0.8f*abs(building.bounds.width);

	float2 previous = outline[0];
	for (unsigned i = 0; i < outline.size(); i++)
	{
		float2 current = outline[i];
		polyFront.push_back(float3(current.x, -current.y, depth/2));
		polyBack.push_back(float3(current.x, -current.y, -depth/2));

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

	float2 uvStart(-building.bounds.width/2.0f, -building.bounds.height/2.0f);
	float2 uvEnd(building.bounds.width/2.0f, building.bounds.height/2.0f);
	Poly f(polyFront);
	f.SetNormals(float3(0, 0, 1), float3(1, 0, 0) * tangScale, float3(0, 1, 0));
	f.SetTexMapping(float3(1, 0, 0), float3(0, 1, 0), uvStart, uvEnd);
	building.polys.push_back(f);
	Poly b(polyBack);
	b.SetNormals(float3(0, 0, -1), float3(1, 0, 0) * tangScale, float3(0, 1, 0)); // Tangents same because tex coords same
	b.SetTexMapping(float3(0, 0, 1), float3(0, 1, 0), uvStart, uvEnd);
	building.polys.push_back(b);

	// Work out the feature polygons
	/*
	for (std::vector<Stroke>::iterator s = processedFeatureOutlines.begin();
		s != processedFeatureOutlines.end(); s++)
	{
		std::vector<int2> stroke = (*s).points;
		featurePolyFront.clear();
		featurePolyBack.clear();
		featurePolySide.clear();
		featurePolyFront.reserve(stroke.size());
		featurePolyBack.reserve(stroke.size());
		featurePolySide.reserve(4);

		// The depth is 20% of the stroke width.
		float featureDepth = 0.1f*abs((*s).bounds.width);

		float2 previous = stroke[0];
		for (unsigned i = 0; i < stroke.size(); i++)
		{
			float2 current = stroke[i];
			featurePolyFront.push_back(float3(current.x, -current.y, featureDepth+depth/2));
			featurePolyBack.push_back(float3(current.x, -current.y, depth/2));

			if (i > 0) {
				featurePolySide.clear();
				featurePolySide.push_back(float3(current.x, -current.y, depth/2));
				featurePolySide.push_back(float3(current.x, -current.y, featureDepth+depth/2));
				featurePolySide.push_back(float3(previous.x, -previous.y, featureDepth+depth/2));
				featurePolySide.push_back(float3(previous.x, -previous.y, depth/2));
				building.polys.push_back(featurePolySide);
			}
			previous = current;
		}
		featurePolyFront.push_back(float3(stroke[0].x, -stroke[0].y, featureDepth+depth/2));
		featurePolyBack.push_back(float3(stroke[0].x, -stroke[0].y, depth/2));

		building.polys.push_back(featurePolyFront);
		building.polys.push_back(featurePolyBack);
	}*/

	building.bounds.depth = (int)depth;
}



void RotateSketch(Building& building, std::vector<int2>& outline, int rotationCount, bool mirrorSketch)
{
	float rotAngle = (float)(D_PI/(rotationCount+1)); // rotate by atleast 90 degrees
	std::vector<float3> polySide;
	polySide.reserve(4);

	// Convert old 2D outline to a 3D outline.			
	std::vector<float3> oldOutline;
	oldOutline.reserve(outline.size());
	for (int i = 0; i < outline.size(); i++)
	{
		float3 point = float3((float)outline[i].x, (float)outline[i].y, 0);
		oldOutline.push_back(point);
	}

	// Mirror the sketch so everything lines up on a central axis
	if (mirrorSketch) {
		for (int i = 0; i < oldOutline.size()/2; i++)
		{
			oldOutline[oldOutline.size()-1-i].x = -oldOutline[i].x;
			oldOutline[oldOutline.size()-1-i].y = oldOutline[i].y;
		}
	}

	for (int j = 0; j < (rotationCount+1)*2; j++)
	{	
		// rotate the old outline by rotAngle to get the new outline.
		std::vector<float3> newOutline = oldOutline;
		for (int i = 0; i < newOutline.size(); i++)
		{
			float temp_x = newOutline[i].z*sin(rotAngle) + newOutline[i].x*cos(rotAngle);
			float temp_z = newOutline[i].z*cos(rotAngle) - newOutline[i].x*sin(rotAngle);
			newOutline[i].x = temp_x;
			newOutline[i].z = temp_z;
		}

		// Build each side polygon
		float3 prevPoint_oldOutline = oldOutline[0];
		float3 prevPoint_newOutline = newOutline[0];
		for (int i = 1; i < newOutline.size(); i++)
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
}


void MirrorSketch(Building& building, vector<int2>& outline)
{
	float2 uvStart(-building.bounds.width/2, -building.bounds.height/2);
	float2 uvEnd(building.bounds.width/2, building.bounds.height/2);
	float tangScale = float(building.bounds.height) / building.bounds.width; // Hack to fix distortion

	// Magic... TODO: Proper comments
	int2 previous = outline[0];
	for (unsigned i = 1; i < outline.size(); i++)
	{
		int2 current = outline[i];

		int yMax = std::max(previous.y, current.y);
		int yMin = std::min(previous.y, current.y);

		std::vector< std::vector<float2> > polys2d = Clip(outline, yMin, yMax);

		if (yMax == yMin) // If min and max are equal we cannot interpolate height to produce a valid polygon
		{
			// Clip has produed a set of degenerate polygons with 4 verts each.
			// Each one of these represents a horizontal rectangle, so we simply find the depth and generate them.
			// We don't have to do both front and side because it's the top, we'd get everything twice.
			// This special case is hilariously bigger than the general algorithm.
			int minZ = std::min(previous.x, current.x);
			int maxZ = std::max(previous.x, current.x);

			for (unsigned p = 0; p < polys2d.size(); p++)
			{
				const std::vector<float2>& poly2d = polys2d[p];
				assert(poly2d.size());

				float minX = poly2d[0].x;
				float maxX = poly2d[0].x;
				for (unsigned i = 1; i < poly2d.size(); i++)
				{
					minX = std::min(minX, poly2d[i].x);
					maxX = std::max(maxX, poly2d[i].x);
				}
				std::vector<float3> polyTop;
				polyTop.reserve(4);
				polyTop.push_back(float3(minX, -yMin, minZ));
				polyTop.push_back(float3(maxX, -yMin, minZ));
				polyTop.push_back(float3(maxX, -yMin, maxZ));
				polyTop.push_back(float3(minX, -yMin, maxZ));
				building.polys.push_back(polyTop);
			}
		}

		for (unsigned p = 0; p < polys2d.size(); p++)
		{
			const std::vector<float2>& poly2d = polys2d[p];

			std::vector<float3> front;
			front.reserve(poly2d.size());
			std::vector<float3> side;
			side.reserve(poly2d.size());
			for (unsigned j = 0; j < poly2d.size(); j++)
			{
				float2 p2d = poly2d[j];
				// Interpolate based on y coordinate
				float z = previous.x + float(p2d.y - previous.y) / (current.y - previous.y) * (current.x - previous.x);
				front.push_back(float3(p2d.x, -p2d.y, z));
				side.push_back(float3(z, -p2d.y, p2d.x));
			}

			float3 frontTangent(1, 0, 0);
			float3 frontBinormal = normal(float3(0.0f, current.y - previous.y, previous.x - current.x));
			if (frontBinormal.y < 0)
				frontBinormal = -frontBinormal;
			float3 frontNormal = normal(float3(0.0f, current.x - previous.x, current.y - previous.y)); // at 90 deg to binormal
			//float3 frontNormal = cross(frontTangent, frontBinormal);
			Poly polyFront(front);
			polyFront.SetNormals(frontNormal, frontTangent * tangScale, frontBinormal);
			polyFront.SetTexMapping(float3(1, 0, 0), float3(0, 1, 0), uvStart, uvEnd);
			building.polys.push_back(polyFront);

			float3 sideTangent(0, 0, 1);
			float3 sideBinormal = normal(float3(current.x - previous.x, previous.y - current.y, 0.0f));
			if (sideBinormal.y < 0)
				sideBinormal = -sideBinormal;
			float3 sideNormal = normal(float3(current.y - previous.y, current.x - previous.x, 0.0f));
			//float3 sideNormal = cross(sideTangent, sideBinormal);
			Poly polySide(side);
			polySide.SetNormals(sideNormal, sideTangent * tangScale, sideBinormal);
			polySide.SetTexMapping(float3(0, 0, 1), float3(0, 1, 0), uvStart, uvEnd);
			building.polys.push_back(polySide);
		}
		previous = current;
	}
}