#pragma once
#include "asset_manager.h"
#include "font.h"

class FontManager : public AssetManager<Font>
{
	virtual Font* _CreateResource(const Array<byte>& data, const String& name, const String& extension) override;

	String _systemPath;

public:
	FontManager() : AssetManager() {}
	virtual ~FontManager() {}

	void SetSystemPath(const String& path) { _systemPath = path; }
};
