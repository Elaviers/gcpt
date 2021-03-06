#include "texture_manager.h"
#include "material_manager.h"
#include "shared/gfx_io.h"
#include "shared/debug.h"
#include "gl.h"
#include <vector>

Texture* TextureManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	if (extension == ".png")
	{
		Texture* tex = IO::ReadPNGFile(data, _maxMipLevels, _maxAnisotropy);

		if (tex == nullptr)
			Debug::Error(CSTR("Could not load texture \"", name, '\"'));

		return tex;
	}

	Texture* tex = Asset::FromText<Texture>(String(data.begin(), data.GetSize()));

	if (tex == nullptr)
		Debug::Error(CSTR("Could not load texture \"", name, '\"'));

	return tex;
}

void TextureManager::_ResourceRead(Texture& texture, const Array<byte>& data, const String& extension)
{
	if (extension == ".png")
	{
		Buffer<byte> pngData;
		unsigned int w, h;
		if (IO::ReadPNGFile(data, pngData, w, h))
		{
			texture.Create(pngData.begin(), w, h, _maxMipLevels, _maxAnisotropy);
		}
	}
	else AssetManager<Texture>::_ResourceRead(texture, data, extension);
}

void TextureManager::Initialise()
{
	byte dataBlack[4] = { 0, 0, 0, 255 };
	byte dataWhite[4] = { 255, 255, 255, 255 };
	byte dataGrey[4] = { 127, 127, 127, 255 };
	byte dataNormalDefault[4] = { 127, 127, 255, 255 };
	
	Texture::Info colourInfo;
	colourInfo.aniso = 1;
	colourInfo.mipLevels = 1;
	colourInfo.minFilter = colourInfo.magFilter = GL_NEAREST;

	_colours.black = new Texture();
	_colours.white = new Texture();
	_colours.grey = new Texture();
	_colours.normalDefault = new Texture();

	_colours.black->info = colourInfo;
	_colours.white->info = colourInfo;
	_colours.grey->info = colourInfo;
	_colours.normalDefault->info = colourInfo;

	_colours.black->Create(dataBlack, 1, 1);
	_colours.white->Create(dataWhite, 1, 1);
	_colours.grey->Create(dataGrey, 1, 1);
	_colours.normalDefault->Create(dataNormalDefault, 1, 1);

	SharedPointerData<Texture>& dBlack = _MapValue("black"), & dWhite = _MapValue("white"), & dGrey = _MapValue("grey"), & dNormalDefault = _MapValue("unitnormal");

	dBlack.SetPtr(_colours.black);
	dWhite.SetPtr(_colours.white);
	dGrey.SetPtr(_colours.grey);
	dNormalDefault.SetPtr(_colours.normalDefault);

	_colours.tBlack = SharedPointer<Texture>(dBlack);
	_colours.tWhite = SharedPointer<Texture>(dWhite);
	_colours.tGrey = SharedPointer<Texture>(dGrey);
	_colours.tNormalDefault = SharedPointer<Texture>(dNormalDefault);
}
