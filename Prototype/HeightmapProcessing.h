#ifndef HEIGHTMAP_PROCESSING_H
#define HEIGHTMAP_PROCESSING_H

#include <SFML/Graphics.hpp>

inline float getHeight(const sf::Uint8* heights, unsigned width, unsigned x, unsigned y)
{
	const sf::Uint8* p = heights + ((y * width) + x) * 4;
	return (*p++ + *p++ + *p) / 3.0f;
}

unsigned char fToChar(float f)
{
	return (int)(f * 127.9999f + 128);
}

inline void setNormal(sf::Uint8* normals, float3 normal, unsigned width, unsigned x, unsigned y)
{
	sf::Uint8* p = normals + ((y * width) + x) * 4;
	*p++ = fToChar(normal.x);
	*p++ = fToChar(normal.y);
	*p++ = fToChar(normal.z);
	*p = 255; // alpha
}

// Height scale is relative to texture coordinates. A Scale of 1.0 indicates that
// the vertical distance between 0 and 255 is the same as the horizontal distance
// between U = 0 to U = 1; As such, heightScale will generally be small.
sf::Image heightToNormal(const sf::Image& heightmap, float heightScale = 0.01f)
{
	// Is there no way to check if heightmap is loaded?

	sf::Vector2u heightMapSize = heightmap.getSize();
	int width = heightMapSize.x;
	int height = heightMapSize.y;
	const sf::Uint8* heightPixels = heightmap.getPixelsPtr();
	//sf::Image normalmap; // SetPixel, like GetPixel performs <i>horribly</i>, so create later
	sf::Uint8* normals = new sf::Uint8[width * height * 4];

	float heightFactor = heightScale / 255.0f;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float refHeight = heightFactor * getHeight(heightPixels, width, x, y);

			// I estimate that since normals in all directions have the same z component (in principal)
			// we can avoid all but the last normalisation. But I have not confirmed this. - dols
			float nx = 0.0f;
			float ny = 0.0f;
			if (x < width - 1)
			{
				float pHeight = heightFactor * getHeight(heightPixels, width, x+1, y);
				nx -= (pHeight - refHeight) * width;
			}
			if (y < height - 1)
			{
				float pHeight = heightFactor * getHeight(heightPixels, width, x, y+1);
				ny -= (pHeight - refHeight) * height;
			}
			if (x > 0)
			{
				float pHeight = heightFactor * getHeight(heightPixels, width, x-1, y);
				nx += (pHeight - refHeight) * width;
			}
			if (y > 0)
			{
				float pHeight = heightFactor * getHeight(heightPixels, width, x, y-1);
				ny += (pHeight - refHeight) * height;
			}
			float3 norm = normal(float3(nx, ny, 1.0f));
			setNormal(normals, norm, width, x, y);
		}
	}

	sf::Image img = sf::Image();
	img.create(width, height, normals);
	return img;
}


#endif // HEIGHTMAP_PROCESSING_H
