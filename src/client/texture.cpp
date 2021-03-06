#include "texture.h"
#include "texture_manager.h"
#include "shared/engine.h"
#include "shared/gfx_io.h"
#include "shared/macro_utils.h"

void Texture::Info::CMD_min(const Array<String>& args)
{
	if (args.GetSize() > 0)
	{
		String string = args[0].ToLower();

		if (string == "nearest")						minFilter = GL_NEAREST;
		else if (string == "nearest_mipmap_nearest")	minFilter = GL_NEAREST_MIPMAP_NEAREST;
		else if (string == "nearest_mipmap_linear")		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		else if (string == "linear")					minFilter = GL_LINEAR;
		else if (string == "linear_mipmap_nearest")		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		else if (string == "linear_mipmap_linear")		minFilter = GL_LINEAR_MIPMAP_LINEAR;
	}

}

void Texture::Info::CMD_mag(const Array<String>& args)
{
	if (args.GetSize() > 0)
	{
		String string = args[0].ToLower();

		if (string == "nearest")		magFilter = GL_NEAREST;
		else if (string == "linear")	magFilter = GL_LINEAR;
	}
}

GLint StringToWrap(const String& stringin)
{
	String string = stringin.ToLower();
	if (string == "repeat") return GL_REPEAT;
	else if (string == "mirror") return GL_MIRRORED_REPEAT;
	else if (string == "clamp") return GL_CLAMP_TO_EDGE;

	return 0;
}

void Texture::Info::CMD_wrap(const Array<String>& args)
{
	if (args.GetSize() == 1)
	{
		GLint wrap = StringToWrap(args[0]);
		if (wrap) wrapx = wrapy = wrap;
	}
	else if (args.GetSize() > 0)
	{
		GLint wrap = StringToWrap(args[0]);
		if (wrap) wrapx = wrap;
		wrap = StringToWrap(args[1]);
		if (wrap) wrapy = wrap;
	}
}

void Texture::_CMD_img(const Array<String>& args)
{
	if (args.GetSize() > 0)
	{
		TextureManager* const textureManager = g_engine.textures;
		if (textureManager)
		{
			const Buffer<String> paths = textureManager->GetPaths();

			for (size_t i = 0; i < paths.GetSize(); ++i)
			{
				String filename = paths[paths.GetSize() - 1 - i] + args[0];

				Buffer<byte> data;
				if (IO::FileExists(filename.begin()) && IO::ReadPNGFile(filename.begin(), data, _width, _height))
				{
					Create(data.begin(), _width, _height, textureManager->GetMaxMipLevels(), textureManager->GetMaxAnisotropy());
					break;
				}
			}
		}
	}
}

const PropertyCollection& Texture::GetProperties()
{
	static PropertyCollection properties;

	IF_FIRST_EXEC_DO({
		properties.Add<byte>("aniso", offsetof(Texture, info.aniso));
		properties.Add<byte>("mips", offsetof(Texture, info.mipLevels));
		properties.AddCommand("min", MemberCommandPtr<Texture::Info>(&Texture::Info::CMD_min), offsetof(Texture, info));
		properties.AddCommand("mag", MemberCommandPtr<Texture::Info>(&Texture::Info::CMD_mag), offsetof(Texture, info));
		properties.AddCommand("wrap", MemberCommandPtr<Texture::Info>(&Texture::Info::CMD_wrap), offsetof(Texture, info));
		properties.AddCommand("img", MemberCommandPtr<Texture>(&Texture::_CMD_img));
	});

	return properties;
}
