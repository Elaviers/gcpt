#include "kickset_manager.h"
#include "debug.h"

KickSet* KickSetManager::_CreateResource(const Array<byte>& data, const String& name, const String& extension)
{
	KickSet* kickSet = new KickSet();
	if (!kickSet->Parse((const char*)data.begin()))
		Debug::Error(CSTR("Error loading KickSet \"", name, "\""));

	return kickSet;
}
