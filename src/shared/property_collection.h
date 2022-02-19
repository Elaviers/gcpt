#pragma once
#include "property.h"
#include "t_buf.h"

class PropertyCollection
{
	Buffer<Property*> _properties;

public:
	PropertyCollection() {}
	PropertyCollection(const PropertyCollection& other);
	PropertyCollection(PropertyCollection&& other) noexcept : _properties(std::move(other._properties)) {}
	~PropertyCollection();

	PropertyCollection& operator=(const PropertyCollection& other);
	PropertyCollection& operator=(PropertyCollection&& other) noexcept;

	const Buffer<Property*>& GetAll() const { return _properties; }

	void Clear();

	String HandleCommand(void* obj, const Array<String>& tokens) const;
	String HandleCommand(void* obj, const String& command) const { return HandleCommand(obj, command.Split<VAllocator<String>>(" ")); }

	void Transfer(const void* from, void* to) const;
	Property* Find(const String& name) const;
	
	template <typename V>
	VariableProperty<V>* FindVar(const String& name) const
	{
		return dynamic_cast<VariableProperty<V>*>(Find(name));
	}

	template <typename V>
	bool Get(const void* obj, const String& name, V& out) const
	{
		VariableProperty<V>* property = FindVar<V>(name);
		if (property) out = property->Get(obj);

		return property != nullptr;
	}

	template <typename V>
	void Set(void* obj, const String& name, const V& value)
	{
		VariableProperty<V>* property = FindVar<V>(name);
		if (property) property->Set(obj, value);
	}

	//////////

	template <typename V>
	void Add(const String& name, size_t offset, byte flags = 0)
	{
		_properties.Add(new OffsetProperty<V>(name, offset, flags));
	}
	
	template <typename T, typename V> void Add(const String& name, const MemberGetter<T, V>& getter,					const MemberSetter<T, V>& setter,			size_t offset = 0, byte flags = 0)	{ _properties.Add(new FptrProperty<T, V>(name, getter, setter, offset, flags)); }
	template <typename T, typename V> void Add(const String& name, const MemberGetter<T, const V&>& getter,				const MemberSetter<T, V>& setter,			size_t offset = 0, byte flags = 0)	{ _properties.Add(new FptrProperty<T, V>(name, getter, setter, offset, flags)); }

	//TODO: Pass-by-value setters!!!

	template <typename T>
	void AddCommand(const String& name, const MemberCommandPtr<T>& function, size_t offset = 0, byte flags = 0)
	{
		_properties.Add(new FunctionProperty<T>(name, function, offset));
	}
};
