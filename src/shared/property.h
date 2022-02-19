#pragma once
#include "t_buf.h"
#include "function.h"
#include "function_member.h"
#include "string.h"
#include "types.h"
#include <typeinfo>

namespace PropertyFlags
{
	enum EPropertyFlags
	{
		READONLY = 0x01,
		MODEL = 0x02,
		MATERIAL = 0x04,
		CLASSID = 0x08,
		DIRECTION = 0x10
	};
}

class Property
{
	String _name;
	String _description;
	
	byte _flags;

	const char* _typeName;

protected:
	Property(const String& name, byte flags, const char* typeName) : _name(name), _flags(flags), _typeName(typeName) {}

public:
	virtual ~Property() {}

	virtual Property* Clone() const = 0;

	byte GetFlags() const { return _flags; }
	const String& GetName() const { return _name; }
	const String& GetDescription() const { return _description; }
	const char* GetTypeName() const { return _typeName; }

	void SetName(const String& name) { _name = name; }
	void SetDescription(const String& description) { _description = description; }

	virtual String GetAsString(const void* obj) const { return ""; }
	virtual void SetAsString(void* obj, const String& value) const {}
	virtual void TransferTo(const void* from, void* to) const {}
	virtual String HandleCommand(void* obj, const Array<String>& args) const 
	{ 
		String combinedArgs;

		for (size_t i = 0; i < args.GetSize(); ++i)
		{
			if (i > 0) combinedArgs += ' ';

			combinedArgs += args[i];
		}

		SetAsString(obj, combinedArgs);
		return GetName() + " : " + GetAsString(obj) + "\n";
	}
};

template <typename T>
class FunctionProperty : public Property
{
	size_t _offset;

	MemberCommandPtr<T> _fptr;

	T& GetSubClass(void* obj) const { return *(T*)((byte*)obj + _offset); }
public:
	FunctionProperty(const String& name, const MemberCommandPtr<T>& function, size_t offset = 0) : Property(name, 0, "function"), _offset(offset), _fptr(function) {}
	virtual ~FunctionProperty() {}

	virtual Property* Clone() const override { return new FunctionProperty(*this); }

	virtual String HandleCommand(void* obj, const Array<String>& tokens) const override { _fptr.Call(GetSubClass(obj), tokens); return ""; }
};

template <typename V>
class VariableProperty : public Property
{
protected:
	VariableProperty(const String& name, byte flags = 0) : Property(name, flags, typeid(V).name()) {}

public:
	virtual ~VariableProperty() { }

	virtual V Get(const void* obj) const = 0;
	virtual void Set(void* obj, const V& value) const = 0;

	virtual String GetAsString(const void* obj) const override { return String::From(Get(obj)); }
	virtual void SetAsString(void* obj, const String& value) const override { Set(obj, value.To<V>()); }
	virtual void TransferTo(const void* from, void* to) const override { Set(to, Get(from)); }
};

template <>
class VariableProperty<CommandPtr> : public Property
{
protected:
	VariableProperty(const String& name, byte flags = 0) : Property(name, flags, typeid(CommandPtr).name()) {}

public:
	virtual ~VariableProperty() { }

	virtual CommandPtr Get(const void* obj) const = 0;
	virtual void Set(void* obj, const CommandPtr& value) const = 0;

	virtual String GetAsString(const void* obj) const override { return "(function)"; }
	virtual String HandleCommand(void* obj, const Array<String>& args) const
	{
		Get(obj)(args);
		return "";
	}
};

template<typename T, typename V>
class FptrProperty : public VariableProperty<V>
{
	size_t _offset;

	const bool _isReferenceGetter;
	union
	{
		MemberGetter<T, const V&>			_getReference;
		MemberGetter<T, V>					_getObject;
	};

	union
	{
		MemberSetter<T, V>					_set;
	};

	T& GetSubClass(void* obj) const { return *(T*)((byte*)obj + _offset); }
	const T& GetSubClass(const void* obj) const { return *(const T*)((byte*)obj + _offset); }

public:
	//YUCK! WTF am I doing?

	FptrProperty(const String& name, const MemberGetter<T, V>& getter, const MemberSetter<T, V>& setter, size_t offset = 0, byte flags = 0) :
		VariableProperty<V>(name, flags), _offset(offset),
		_isReferenceGetter(false),
		_getObject(getter), _set(setter)
	{}

	FptrProperty(const String& name, const MemberGetter<T, const V&>& getter, const MemberSetter<T, V>& setter, size_t offset = 0, byte flags = 0) :
		VariableProperty<V>(name, flags), _offset(offset),
		_isReferenceGetter(true),
		_getReference(getter), _set(setter)
	{}

	virtual ~FptrProperty() {}

	virtual Property* Clone() const override { return new FptrProperty(*this); }

	virtual V Get(const void* obj) const override 
	{
		return _isReferenceGetter ? _getReference.Call(GetSubClass(obj)) : _getObject.Call(GetSubClass(obj)); 
	}
	
	virtual void Set(void* obj, const V& value) const override 
	{
		if ((Property::GetFlags() & PropertyFlags::READONLY) == 0)
		{
			_set.Call(GetSubClass(obj), value);
		}
	}
};

template<typename V>
class OffsetProperty : public VariableProperty<V>
{
	size_t _offset;

public:
	OffsetProperty(const String& name, size_t offset, byte flags = 0) : VariableProperty<V>(name, flags), _offset(offset) {}

	virtual ~OffsetProperty() {}

	virtual Property* Clone() const override { return new OffsetProperty(*this); }

	const V& GetRef(const void* obj) const
	{
		const V* ptr = (const V*)((const byte*)obj + _offset);
		return *ptr;
	}

	virtual V Get(const void* obj) const override
	{
		return GetRef(obj);
	}

	virtual void Set(void* obj, const V& value) const override
	{
		if ((Property::GetFlags() & PropertyFlags::READONLY) == 0)
		{
			V* ptr = (V*)((byte*)obj + _offset);
			*ptr = value;
		}
	}
};
