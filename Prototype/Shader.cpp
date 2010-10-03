//#include <GL\glew.h>

#include "Shader.h"

#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <iostream>

// Shader stuff done with assistance of http://gpwiki.org/index.php/OpenGL:Codes:Simple_GLSL_example

void printLog(GLuint obj)
{
	int infologLength = 0;
	int maxLength;
	
	if(glIsShader(obj))
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	else
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
			
	std::string infoLog;
	infoLog.resize(maxLength);
 
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, maxLength, &infologLength, &infoLog[0]);
	else
		glGetProgramInfoLog(obj, maxLength, &infologLength, &infoLog[0]);
 
	if (infologLength > 0)
		std::cout << infoLog;
}

std::string fileToStr(std::string file)
{
	std::ifstream textstream(file.c_str());
	std::vector<std::string> text;
	std::string line;
	while (getline(textstream, line))
	{
		text.push_back(line + "\n");
	}
	std::string alltext;
	for (unsigned i = 0; i < text.size(); i++)
		alltext += text[i];
	return alltext;
}

Shader::Shader(std::string vertFile, std::string fragFile)
{
	sf::priv::GraphicsContext ctx; // Make sure glew is initialised

	std::string vsSource = fileToStr(vertFile); // Keep this in scope until you're done with the raw stringS
	const char* vsSourceChar = vsSource.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vsSourceChar, NULL); 
	glCompileShader(vs);
	printLog(vs);

	std::string fsSource = fileToStr(fragFile);
	const char* fsSourceChar = fsSource.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fsSourceChar, NULL);
	glCompileShader(fs);
	printLog(fs);

	shaderProgram = glCreateProgram();
	glAttachObjectARB(shaderProgram, vs);
	glAttachObjectARB(shaderProgram, fs);
	glLinkProgramARB(shaderProgram);
	printLog(shaderProgram);
}

void Shader::Enable(bool enable)
{
	if (enable)
		glUseProgramObjectARB(shaderProgram);
	else
		glUseProgramObjectARB(0);
}