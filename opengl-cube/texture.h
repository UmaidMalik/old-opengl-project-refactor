#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <stb_image/stb_image.h>

class Texture
{
public:

	Texture(const char* textureFilePath);

	~Texture();

	unsigned int getTextureID();

private:
	unsigned int textureID;
};
#endif 

