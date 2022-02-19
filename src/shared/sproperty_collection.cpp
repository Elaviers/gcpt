#include "sproperty_collection.h"

SPropertyCollection::~SPropertyCollection()
{
	for (SProperty* property : _properties)
		delete property;
}

void SPropertyCollection::Clear()
{
	for (SProperty* property : _properties)
		delete property;

	_properties.Clear();
}

SProperty* SPropertyCollection::Find(const String& name) const
{
	for (size_t i = 0; i < _properties.GetSize(); ++i)
		if (_properties[i]->GetName().Equals(name, true))
			return _properties[i];

	return nullptr;
}

String SPropertyCollection::HandleCommand(const Array<String>& tokens) const
{
	if (tokens.GetSize() > 0)
	{
		SProperty* property = Find(tokens[0].ToLower());

		if (property)
		{
			VBuffer<String> args(&tokens[1], tokens.GetSize() - 1);
			return property->HandleCommand(args);
		}
	}

	return "";
}