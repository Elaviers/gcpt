#include "text_manager.h"

TextProvider* TextManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	TextProvider* provider = new TextProvider();
	provider->Set(String(data.begin(), data.GetSize()));
	return provider;
}

void TextManager::_ResourceRead(TextProvider& provider, const Array<byte>& data, const String& extension)
{
	provider.Set(String(data.begin(), data.GetSize()));
}
