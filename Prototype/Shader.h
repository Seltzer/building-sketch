#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader
{
public:
	Shader(std::string vertFile, std::string fragFile);
	void Enable(bool enable=true);
private:
	int shaderProgram;
};

#endif //SHADER_H
