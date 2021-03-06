#include "gl_texture.h"

void GLTexture::Create(GLsizei width, GLsizei height, const GLvoid *data, GLint mipLevels, GLfloat aniso, GLint min, GLint mag, GLint wrapx, GLint wrapy)
{
	glGenTextures(1, &_id);

	glBindTexture(GL_TEXTURE_2D, _id);

	if (mipLevels > 1)
	{
		glTexStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA8, width, height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapx);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
}

void GLTexture::Modify(GLint mip, GLsizei x, GLsizei y, GLsizei width, GLsizei height, const GLvoid* data)
{
	if (_id)
	{
		glBindTexture(GL_TEXTURE_2D, _id);
		glTexSubImage2D(GL_TEXTURE_2D, mip, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
}
