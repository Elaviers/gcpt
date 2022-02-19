#pragma once
#include "function.h"

enum EConVarFlags : int32
{
	ECVAR_REPLICATE = 1 << 1,
	ECVAR_CHEAT = 1 << 2,


};

class ConVar
{
	String _name;
	String _hint;
	EConVarFlags _flags;
	Function<void> _onChanged;

	struct
	{
		String vstr;
		float vflt;
		int32 vint;
	} _value;

	friend void Con_ExecuteCommand(const char*);

public:
	ConVar(const String& name, const String& defaultValue, const String& hint, EConVarFlags flags = (EConVarFlags)0, const Function<void>& onChanged = Function<void>());
	~ConVar();

	constexpr const String& GetName() const noexcept { return _name; }
	constexpr const String& GetHint() const noexcept { return _hint; }
	constexpr EConVarFlags GetFlags() const noexcept { return _flags; }

	constexpr const String& GetString() const noexcept { return _value.vstr; }
	constexpr const float GetFloat() const noexcept { return _value.vflt; }
	constexpr const int32 GetInt() const noexcept { return _value.vint; }
	constexpr const bool GetBool() const noexcept { return _value.vint != 0; }

	void SetString(const String& value);
	void SetFloat(float value);
	void SetInt(int32 value);
	void SetBool(bool value);
};

class ConArgs
{
	String _commandString;
	Buffer<String> _args;

public:
	ConArgs(const String& cmd, const Buffer<String>& args) : _commandString(cmd), _args(args) {}

	constexpr const String& GetCommandString() const noexcept { return _commandString; }
	constexpr const String& GetArg(int index) const noexcept { return _args[index]; }
	constexpr int GetArgCount() const noexcept { return (int)_args.GetSize(); }
};

class ConCommand
{
	String _name;
	String _hint;
	Function<void, const ConArgs&> _command;

	friend void Con_ExecuteCommand(const char*);

public:
	ConCommand(const String& name, const String& hint, const Function<void, const ConArgs&>& fptr);
	~ConCommand();

	constexpr const String& GetName() const noexcept { return _name; }
	constexpr const String& GetHint() const noexcept { return _hint; }
};

void Con_ExecuteCommand(const char* command);
