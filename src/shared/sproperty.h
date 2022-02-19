#pragma once
#include "property.h"

class _DummyProperty : public Property
{
public:
	_DummyProperty(const String& name, byte flags, const char* typeName) : Property(name, flags, typeName) {}
	virtual ~_DummyProperty() {}

	virtual Property* Clone() const override { return new _DummyProperty(*this); }
};

class SProperty
{
protected:
	void* const _object;
	Property& _baseProperty;

	SProperty(void* object, Property& property) : _object(object), _baseProperty(property) {}

public:
	virtual ~SProperty() {}

	byte GetFlags() const { return _baseProperty.GetFlags(); }
	const String& GetName() const { return _baseProperty.GetName(); }
	const String& GetDescription() const { return _baseProperty.GetDescription(); }
	const char* GetTypeName() const { return _baseProperty.GetTypeName(); }

	void SetName(const String& name) { return _baseProperty.SetName(name); }
	void SetDescription(const String& description) { return _baseProperty.SetDescription(description); }

	String GetAsString() const { return _baseProperty.GetAsString(_object); }
	void SetAsString(const String& string) const { _baseProperty.SetAsString(_object, string); }
	String HandleCommand(const Array<String>& args) const { return _baseProperty.HandleCommand(_object, args); }
};

template<typename T>
class SVariableProperty : public SProperty
{
	const VariableProperty<T>& _Property() const { return (const VariableProperty<T>&)_baseProperty; }

protected:
	SVariableProperty(void* object, VariableProperty<T>& property) : SProperty(object, property) {}

public:
	virtual ~SVariableProperty() {}

	virtual T Get() const { return _Property().Get(_object); }
	virtual void Set(const T& value) const { return _Property().Set(_object, value); }
};

template<typename T>
class SFptrProperty : public SVariableProperty<T>
{
	_DummyProperty _property;

	const bool _isReferenceGetter;
	union
	{
		Getter<const T&>			_getReference;
		Getter<T>					_getObject;
	};

	union
	{
		Setter<T>					_set;
	};

public:
	//		:/
	SFptrProperty(const String& name, const Getter<T>& getter, const Setter<T>& setter, byte flags) : 
		SVariableProperty<T>(nullptr, _property), _property(name, flags, typeid(T).name()),
		_isReferenceGetter(false),
		_getObject(getter), _set(setter)
	{}

	SFptrProperty(const String& name, const Getter<const T&>& getter, const Setter<T>& setter, byte flags) :
		SVariableProperty<T>(nullptr, _property), _property(name, flags, typeid(T).name()),
		_isReferenceGetter(true),
		_getReference(getter), _set(setter)
	{}

	virtual ~SFptrProperty() {};

	virtual T Get() const override
	{
		return _isReferenceGetter ? _getReference() : _getObject(); 
	}

	virtual void Set(const T& value) const override 
	{ 
		_set(value); 
	}
};

template<typename T>
class SReferenceProperty : public SVariableProperty<T>
{
	OffsetProperty<T> _property;

public:
	SReferenceProperty(const String& name, const T& value, byte flags = 0) : 
		SVariableProperty<T>(nullptr, _property), 
		_property(name, (size_t)& value, flags) 
	{}
	
	virtual ~SReferenceProperty() {}
};

template<typename T>
class SValueProperty : public SVariableProperty<T>
{
	OffsetProperty<T> _property;
	T _value;

public:
	SValueProperty(const String& name, const T& value, byte flags = 0) : 
		SVariableProperty<T>(nullptr, _property), 
		_property(name, (size_t)&_value, flags), 
		_value(value) 
	{}

	virtual ~SValueProperty() {}
};
