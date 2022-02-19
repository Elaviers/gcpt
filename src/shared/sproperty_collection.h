#pragma once
#include "sproperty.h"
#include "t_buf.h"

class SPropertyCollection
{
	Buffer<SProperty*> _properties;

public:
	SPropertyCollection() {}
	SPropertyCollection(const SPropertyCollection&) = delete;
	SPropertyCollection(SPropertyCollection&& other) noexcept : _properties(std::move(other._properties)) {}

	~SPropertyCollection();

	SPropertyCollection& operator=(const SPropertyCollection&) = delete;
	SPropertyCollection& operator=(SPropertyCollection&& other) noexcept { Clear(); _properties = std::move(other._properties); return *this; }

	const Buffer<SProperty*>& GetAll() const { return _properties; }

	void Clear();

	String HandleCommand(const Array<String>& tokens) const;
	String HandleCommand(const String& command) const { return HandleCommand(command.Split<VAllocator<String>>(" ")); }

	SProperty* Find(const String& name) const;
	
	template <typename T>
	SVariableProperty<T>* FindVar(const String &name)
	{
		return dynamic_cast<SVariableProperty<T>*>(Find(name));
	}

	template <typename T>
	bool Get(const String& name, T& out) const
	{
		SVariableProperty<T>* property = FindVar<T>(name);
		if (property) out = property->Get();

		return property != nullptr;
	}

	template <typename T>
	void Set(const String& name, const T& value)
	{
		SVariableProperty<T>* property = FindVar<T>(name);
		if (property) property->Set(value);
	}

	//////////

	template <typename T>
	SValueProperty<T>* CreateVar(const String& name, const T& value, byte flags = 0)
	{
		SValueProperty<T>* const property = new SValueProperty<T>(name, value, flags);
		_properties.Add(property);
		return property;
	}

	template <typename T>
	SReferenceProperty<T>* Add(const String &name, T& value, byte flags = 0)
	{
		SReferenceProperty<T>* const property = new SReferenceProperty<T>(name, value, flags);
		_properties.Add(property);
		return property;
	}

	template <typename T, typename V> void Add(const String& name, const MemberGetter<T, V>& getter, const MemberSetter<T, V>& setter, byte flags = 0) { _properties.Add(new SFptrProperty<T, V>(name, getter, setter, flags)); }
	template <typename T, typename V> void Add(const String& name, const MemberGetter<T, const V&>& getter, const MemberSetter<T, V>& setter, byte flags = 0) { _properties.Add(new SFptrProperty<T, V>(name, getter, setter, flags)); }
};
