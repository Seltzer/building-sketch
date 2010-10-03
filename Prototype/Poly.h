#ifndef POLY_H
#define POLY_H

class Poly
{
public:
	Poly(std::vector<float3> verts) : verts(verts)
	{}
	float3& operator[](int i)
	{
		return verts[i];
	}
	const float3& operator[](int i) const
	{
		return verts[i];
	}
	const std::vector<float3>& GetVerts() const
	{
		return verts;
	}
	void SetNormals(float3 normal, float3 tangent, float3 bitangent)
	{
		this->normal = normal;
		this->tangent = tangent;
		this->bitangent = bitangent;
	}
	float3 GetNormal() const {return normal;}
	float3 GetTangent() const {return tangent;}
	float3 GetBitangent() const {return bitangent;}

	// Texture mapping is assumed to be uniform, tangent and bitangent define the orientation
	// of the mapping already, we just need to know how far along the axis the texture starts and ends
	void SetTexMapping(float2 start, float2 end)
	{
		this->start = start;
		this->end = end;
	}
	float2 GetTexCoords(float3 pos) const
	{
		float2 scale = end - start;
		float2 projected(dot(pos, tangent), dot(pos, bitangent)); // Convert to texture space
		return (projected - start) / scale; // Rescale
	}
private:
	std::vector<float3> verts;
	float3 normal;
	float3 tangent;
	float3 bitangent;
	float2 start;
	float2 end;
};


#endif //POLY_H
