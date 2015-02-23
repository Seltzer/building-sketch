#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "Types.h"
#include "BuildingSketch.h"
#include <iostream>

int main()
{
	BuildingSketch app;
	app.RenderLoop();
	
	return 0;
}