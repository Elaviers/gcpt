#pragma once
#include "asset_manager.h"
#include "KickSet.h"

class KickSetManager : public AssetManager<KickSet>
{
	virtual KickSet* _CreateResource(const Array<byte>& data, const String& name, const String& extension);

};
