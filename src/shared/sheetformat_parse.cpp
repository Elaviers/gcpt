#include "sheetformat.h"
#include "t_stack.h"
#include "iterator_utils.h"
#include "debug.h"

static void ParseError(const char* str)
{
	Debug::PrintLine(CSTR("[SpritesheetParser] ", str));

}

enum class Mode
{
	NONE,
	MACRO,
	DO,
	SET,
	GOTO,
	STAMPAREA,
	STAMPDEF,
	STAMP,
	DEF,

	_COUNT
};

const char* modeNames[] = 
{
	" (NONE) ",
	"macro",
	"do",
	"set",
	"goto",
	"stamp_area",
	"stamp_def",
	"stamp",
	"def",

	" (INVALID) "
};

struct Parser
{
	SheetFormat* target;

	Stack<SpriteSet*> setStack;
	SpriteSet* currentSet;

	Hashmap<String, String> macros;

	uint16 curX = 0, curY = 0;
	uint16 stampW = 1, stampH = 1;
	bool stampDirIsY = false; //true == y, false == x

	uint16 stampAreaStartX = 0, stampAreaStartY = 0, stampAreaEndX = 2048, stampAreaEndY = 2048;

	Parser(SheetFormat* target) noexcept : target(target), setStack(8)
	{
		currentSet = setStack.Push(&target->sprites);
	}

	bool Parse(const char* string, size_t length)
	{
		constexpr int MAX_ARGS = 16;

		Mode currentMode = Mode::NONE;
		
		String args[MAX_ARGS] = {};
		int argCount = 0;

		bool done = false;
		const char* c = string;

		const SpriteSet* const startingSet = currentSet;

		while (true)
		{
			while (true)
			{
				if (c - string >= length)
				{
					done = true;
					break;
				}

				if (*c != ' ' && *c != '\t')
					break;

				++c;
			}

			if (done || *c == '\r' || *c == '\n' || *c == ';')
			{
				//exec statement

				switch (currentMode)
				{
				case Mode::MACRO:
				case Mode::SET:
					if (*c == ';')
					{
						ParseError("Cannot execute \"macro\" or \"set\" statements! These are only used for declaring scopes!");
						return false;
					}

					break; //ok, we're waiting for a scope

				case Mode::DO:
					if (argCount > 0)
					{
						String* macro = macros.TryGet(args[1]);

						if (macro == nullptr)
						{
							ParseError(CSTR("Cannot execute macro \'", args[1], "\" because it doesn't exist!"));
							return false;
						}

						if (!Parse(macro->begin(), macro->GetLength()))
						{
							ParseError(CSTR("Failed during execution of \'", args[1], "\""));
							return false;
						}
					}
					else
					{
						ParseError("\"do\" requires an argument");
						return false;
					}
					break;

				case Mode::GOTO:
					if (argCount > 2)
					{
						curX = args[1].ToInt();
						curY = args[2].ToInt();
					}
					else
					{
						ParseError("\"goto\" requires 2 arguments");
						return false;
					}
					break;

				case Mode::STAMPAREA:
					if (argCount > 4)
					{
						stampAreaStartX = args[1].ToInt();
						stampAreaStartY = args[2].ToInt();
						stampAreaEndX = stampAreaStartX + args[3].ToInt();
						stampAreaEndY = stampAreaStartY + args[4].ToInt();
					}
					else
					{
						ParseError("\"stamp_area\" requires 4 arguments");
						return false;
					}
					break;

				case Mode::STAMPDEF:
					if (argCount > 2)
					{
						stampW = args[1].ToInt();
						stampH = args[2].ToInt();

						if (argCount > 3)
							stampDirIsY = (args[3].Equals("y", true));
					}
					else
					{
						ParseError("\"stamp_def\" requires at least 2 arguments");
						return false;
					}
					break;

				case Mode::STAMP:
					if (argCount > 1)
					{
						SpriteDef& sprite = currentSet->sprites[args[1]];
						sprite.x = curX;
						sprite.y = curY;

						if (stampW < 0)
						{
							sprite.x = curX - stampW;
							sprite.w = -stampW;
						}
						else
						{
							sprite.x = curX;
							sprite.w = stampW;
						}

						if (stampH < 0)
						{
							sprite.y = curY - stampH;
							sprite.h = -stampH;
						}
						else
						{
							sprite.y = curY;
							sprite.h = stampH;
						}
					}

					if (stampW < 0)
					{
						if (-stampW > curX)
							curX = stampAreaEndX;
						else if ((curX -= -stampW) < stampAreaStartX)
							curX = stampAreaEndX;
					}
					else if ((curX += stampW) > stampAreaEndX)
						curX = stampAreaStartX;

					if (stampH < 0)
					{
						if (-stampH > curY)
							curY = stampAreaEndY;
						else if ((curY -= -stampH) < stampAreaStartY)
							curY = stampAreaEndY;
					}
					else if ((curY += stampH) > stampAreaEndY)
						curY = stampAreaStartY;

					break;
					
				case Mode::DEF:
					if (argCount > 5)
					{
						SpriteDef& sprite = currentSet->sprites[args[1]];
						sprite.x = args[2].ToInt();
						sprite.y = args[3].ToInt();
						sprite.w = args[4].ToInt();
						sprite.h = args[5].ToInt();
					}
					else
					{
						ParseError("\"def\" requires at least 5 arguments");
						return false;
					}
					break;
				}

				if (currentMode != Mode::SET && currentMode != Mode::MACRO)
				{
					//If we're not waiting for a scope, clear our state now
					currentMode = Mode::NONE;
					argCount = 0;
				}

				++c;
			}
			else if (*c == '{')
			{
				//open scope
				
				const Mode mode = currentMode;
				currentMode = Mode::NONE;
				switch (mode)
				{
				case Mode::MACRO:
					if (argCount > 1)
					{
						argCount = 0;

						int macroScope = 1;
						const char* macroStart = c + 1;

						while (true)
						{
							if (*c == '\0') break;

							if (*c++ == '}')
								if (--macroScope <= 0)
									break;
						}

						if (macroScope == 0)
							macros.Set(args[1], String(macroStart, c - macroStart - 1));
						else
							ParseError(CSTR("Macro \"", args[1], "\" is invalid"));

						continue;
					}
					
					ParseError("Missing macro name");
					return false;

				case Mode::SET:
					if (argCount > 1)
					{
						SpriteSet* newSet = &currentSet->subsets[args[1]];
						currentSet = setStack.Push(newSet);

						++c;
						continue;
					}

					ParseError("Missing set name");
					return false;
				}

				ParseError("Invalid context for \'{\'");
				return false;
			}
			else if (*c == '}')
			{
				//close scope
				currentMode = Mode::NONE;
				argCount = 0;

				if (setStack.IsEmpty())
					ParseError("Invalid context for \'}\'.. the spriteset stack is empty????");
				else
				{
					SpriteSet** pop = setStack.Pop();
					if (pop == nullptr)
					{
						ParseError("Cannot pop root set! You have too many closing brackets!");
						return false;
					}

					currentSet = *pop;
					++c;
					continue;
				}

				return false;
			}
			else
			{
				//word
				const char* wordStart = c;

				while (true)
				{
					if (*c == ' ' ||
						*c == '\t' ||
						*c == '\r' ||
						*c == '\n' ||
						*c == ';')
						break;

					if (*c == '\0')
					{
						done = true;
						break;
					}

					++c;
				}


				if (c > wordStart)
				{
					String word(wordStart, c - wordStart);
					if (argCount == 0)
					{
						for (size_t i = 0; i < (size_t)Mode::_COUNT; ++i)
						{
							if (StringUtils::Equal_CaseInsensitive(word.begin(), modeNames[i]))
							{
								currentMode = (Mode)i;
								argCount = 0;
								break;
							}
						}
					}

					args[argCount++] = std::move(word);
				}
			}
			
			if (done) break;
		}

		if (currentSet != startingSet)
		{
			ParseError("File ended inside a set! You don't have enough closing brackets! Popping stack until back to starting set...");
			while (*setStack.Pop() != startingSet);
		}

		return true;
	}
};

bool SheetFormat::Parse(const String& string)
{
	Parser parser(this);
	return parser.Parse(string.begin(), string.GetLength());
}
