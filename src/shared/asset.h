#pragma once
#include "t_buf.h"
#include "string.h"
#include "property_collection.h"

class ByteReader;
class ByteWriter;

enum EAssetID
{
	ASSET_TEXTURE			= 1,
	ASSET_MATERIAL_SURFACE	= 10,
	ASSET_MATERIAL_GRID		= 11,
	ASSET_MATERIAL_FONT		= 12,
	ASSET_MESH_STATIC		= 20,
	ASSET_MESH_SKELETAL		= 21,
	ASSET_MODEL				= 30,
	ASSET_ANIMATION			= 40
};

class Asset
{
protected:
	Asset() {}

public:
	virtual ~Asset() {}

	virtual const PropertyCollection& GetProperties() 
	{ 
		static PropertyCollection properties; 
		return properties;
	}
	
	//T Must be derived from Asset
	template<typename T>
	static T* FromText(const String &string)
	{
		Asset* ass = new T();
		ass->ReadText(string);
		return (T*)ass;
	}

	String GetAsText() const { return WriteText(); }

	Buffer<byte> GetAsData() const;

	virtual void ReadText(const String& string)
	{
		Buffer<String> lines = string.ToLower().Split("\r\n");
		for (const String& line : lines)
			String unused = GetProperties().HandleCommand(this, line);
	}

	virtual String WriteText() const { return ""; }

	virtual void Read(ByteReader&) {}
	virtual void Write(ByteWriter&) const {}
};
