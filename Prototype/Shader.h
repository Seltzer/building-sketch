#ifndef SHADER_H
#define SHADER_H

#include <string>

namespace sf {
	class Image;
}

class Shader
{
public:
	Shader(std::string vertFile, std::string fragFile);
	void Enable(bool enable=true);
	void BindTexture(const sf::Image& image, std::string samplerName, int texUnit);
private:
	int shaderProgram;
};

#endif //SHADER_H
