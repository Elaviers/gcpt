#pragma once
#include "shared/t_buf.h"
#include "gl.h"

struct TextureData
{
	const byte* data;
	unsigned int width;
	unsigned int height;
};

class GLCubemap
{
private:
	GLuint _id;

public:
	GLCubemap() : _id(0) {}
	GLCubemap(const GLCubemap&) = delete;
	GLCubemap(GLCubemap&& other) noexcept : _id(other._id) { other._id = 0; }
	~GLCubemap() { glDeleteTextures(1, &_id); }

	void Create(TextureData faces[6]);

	void Bind(byte unit) const { glActiveTexture(GL_TEXTURE0 + unit); glBindTexture(GL_TEXTURE_CUBE_MAP, _id); }

	GLuint getGLId() const { return _id; }
};
