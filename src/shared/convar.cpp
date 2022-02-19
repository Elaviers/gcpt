#include "convar.h"
#include "console.h"
#include "t_hashmap.h"
#include "maths.h"

struct ConItem
{
	union
	{
		ConVar* var;
		ConCommand* cmd;
	} ptr;

	bool isVar;
};

static Hashmap<String, ConItem>& _ConItems()
{
	static Hashmap<String, ConItem> sConItems;
	return sConItems;
}

ConVar::ConVar(const String& name, const String& defaultValue, const String& hint, EConVarFlags flags, const Function<void>& onChanged)
	: _name(name), _hint(hint), _flags(flags), _onChanged(onChanged)
{
	SetString(defaultValue);

	ConItem& item = _ConItems()[_name.ToLower()];
	item.isVar = true;
	item.ptr.var = this;
}

ConVar::~ConVar()
{
	_ConItems()[_name.ToLower()].ptr.var = nullptr;
}

ConVar sv_cheats("sv_cheats", "0", "", ECVAR_REPLICATE);

static bool CanChangeConvar(const ConVar& cvar)
{
	if (!sv_cheats.GetBool() && (cvar.GetFlags() & ECVAR_CHEAT))
		return false;


}

void ConVar::SetString(const String& value)
{
	if (value != _value.vstr)
	{
		_value.vstr = value;
		_value.vflt = value.ToFloat();
		_value.vint = value.ToInt();
		_onChanged();
	}
}

void ConVar::SetFloat(float value)
{
	if (value != _value.vflt)
	{
		_value.vstr = String::FromFloat(value);
		_value.vflt = value;
		_value.vint = (int32)value;
		_onChanged();
	}
}

void ConVar::SetInt(int32 value)
{
	if (value != _value.vint)
	{
		_value.vstr = String::FromInt(value);
		_value.vflt = (float)value;
		_value.vint = value;
		_onChanged();
	}
}

void ConVar::SetBool(bool value)
{
	if ((_value.vint != 0) != value)
	{
		SetInt(value ? 1 : 0);
	}
}

ConCommand::ConCommand(const String& name, const String& hint, const Function<void, const ConArgs&>& fptr) : _name(name), _hint(hint), _command(fptr)
{
	ConItem& item = _ConItems()[name.ToLower()];
	item.isVar = false;
	item.ptr.cmd = this;
}

ConCommand::~ConCommand()
{
	_ConItems()[_name.ToLower()].ptr.cmd = nullptr;
}

void Con_ExecuteCommand(const char* cmd)
{
	String cmdString = String(cmd);
	Buffer<String> commands = cmdString.Split(";\n\r");

	for (String& command : commands)
	{
		String commandFull = command;
		Buffer<String> args;
		char* argStart = &command[0];

		bool isInQuotes = false;
		for (char* c = argStart; *c;)
		{
			if (!isInQuotes && (*c == ' ' || *c == '\t'))
			{
				if (c > argStart)
				{
					*c = '\0';
					
					do ++c; while (*c == ' ' || *c == '\t');

					if (*c)
					{
						args.Add(argStart);
						argStart = c;
					}
				}

				continue;
			}

			if (*c == '\"')
			{
				if (c == argStart)
				{
					isInQuotes = !isInQuotes;

					if (isInQuotes)
						++argStart;
				}
				else if (c[-1] != '\\')
				{
					if (isInQuotes)
					{
						*c = '\0';
						isInQuotes = false;
					}
					else
					{
						Console_Msg("Malformed command - opening quotes midway through arg (%s)\n", commandFull.begin());
						return;
					}
				}
			}

			++c;
		}

		args.Add(argStart);

		if (args.GetSize() == 0) //First arg is an incomplete quoted string..
		{
			Console_Msg("Invalid args\n");
			return;
		}

		ConItem* item = _ConItems().TryGet(args[0].ToLower());

		if (item)
		{
			if (item->isVar)
			{
				if (!item->ptr.var)
					goto invalidcommand;

				if (args.GetSize() > 1)
					item->ptr.var->SetString(args[1]);
				else
					Console_Msg("%s: %s\n", item->ptr.var->GetName().begin(), item->ptr.var->GetString().begin());
			}
			else
			{
				if (!item->ptr.cmd)
					goto invalidcommand;

				item->ptr.cmd->_command(ConArgs(commandFull, args));
			}

			return;
		}
		
		invalidcommand:
		Console_Msg("\"%s\" is not a valid console command\n");
	}	
}

ConCommand cc_list("list", "List console commands", [](const ConArgs& args) {
	
	const auto& conItems = _ConItems();

	for (const Pair<const String, ConItem>& item : conItems)
	{
		const String& hint = item.second.isVar ? item.second.ptr.var->GetHint() : item.second.ptr.cmd->GetHint();

		Console_Msg("[%s] %s\t\t%s\n", item.second.isVar ? "var" : "cmd", item.first.begin(), hint.begin());

	}
});
