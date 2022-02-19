#pragma once
#include "asset_manager.h"
#include "text_provider.h"

class TextManager : public AssetManager<TextProvider>
{
	virtual TextProvider* _CreateResource(const Array<byte>& data, const String& name, const String& extension) override;
	virtual void _ResourceRead(TextProvider&, const Array<byte>&, const String&) override;

public:
	TextManager() : AssetManager<TextProvider>() {}
	virtual ~TextManager() {}
};
