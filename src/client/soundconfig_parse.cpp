#include "sound.h"
#include "shared/engine.h"
#include "shared/t_stack.h"
#include "shared/iterator_utils.h"
#include "shared/debug.h"

const char* g_soundTypeNames[(size_t)ESoundEvent::_COUNT] =
{
	"",
	"start",
	"levelup",
	"leveldn",
	"win",
	"lose",
	"clear",
	"allclear",

	"tetris_back2back",
	"tetris_tspin",

	"puyo_combo",

	"lock",
	"rotleft",
	"rotright",
	"rotdouble",
	"movleft",
	"movright",
	"drop",
	"softdrop",
	"harddrop",

	"ui_nav_up",
	"ui_nav_dn",
	"ui_switch_tab",
	"ui_confirm",
	"ui_cancel",
	"ui_pause",

	"mus_bg",
	"mus_danger",
};

static void ParseError(const char* error)
{
	Debug::PrintLine(CSTR("[SOUNDCONFIG] ", error));
}

constexpr static bool IsBracket(char c) noexcept
{
	return c == '[' || c == '(' || c == '{' || c == ']' || c == ')' || c == '}';
}

static String QualifySound(const String& dir, const String& sound)
{
	if (sound.GetLength() > 0 && sound[0] == '/')
		return sound.SubString(1);

	return dir + sound;
}

bool SoundConfig::Parse(const String& name, const char* string)
{
	const char* c = string;

	Stack<SoundBucket*> bucketStack(8);
	SoundBucket* bucket = nullptr;

	const int maxArgs = 16;
	String args[maxArgs] = {};
	int argCount = 0;

	const String curDir = Utilities::DirectoryOf(name) + '/';

	SoundConditional cond;

	while (true)
	{
		while (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n' || *c == ';')
			++c;

		if (*c == '\0')
			break;
		
		if (*c == '(')
		{
			const char* openBracket = c;

			while (*c != '\0' && *c != ')') ++c;

			args[argCount++] = String(openBracket, c - openBracket);

			if (argCount > maxArgs)
			{
				ParseError("Too many arguments!");
				return false;
			}
		}
		else if (*c == ')')
		{
			ParseError("\')\' encountered before suitable opening bracket");
			return false;
		}
		else if (*c == '[')
		{
			++c;
			const char* startOfCondition = c;
			while (true)
			{
				if (*c == '\0' || *c == '\r' || *c == '\n')
				{
					ParseError("condition ended prematurely!");
					startOfCondition = nullptr;
					break;
				}

				if (*c == ']')
					break;

				++c;
			}
			++c;

			if (startOfCondition)
			{
				Buffer<String> statements = String(startOfCondition, c - startOfCondition - 1).Split(" \t\r\n; ");
				for (const String& statement : statements)
				{
					String k, v;

					const char* opStart;
					const char* opStop;
					for (opStart = statement.begin(); *opStart && *opStart != '=' && *opStart != '<' && *opStart != '>' && *opStart != '!'; ++opStart);
					for (opStop = opStart; *opStop == '=' || *opStop == '<' || *opStop == '>' || *opStop == '!'; ++opStop);

					if (opStop - opStart > 2)
					{
						Debug::Error("Comparison operators with more than 2 chars not supported");
						continue;
					}

					if (opStart == statement.begin())
					{
						if (opStop - opStart > 1 || *opStart != '!')
						{
							ParseError("conditional began with an invalid operator. conditions may only start with the '!' operator.");
							continue;
						}

						k = statement;
						cond.comparison = SoundConditional::Comparison::IS_NOT_SET;
					}
					else if (opStart == opStop)
					{
						k = statement;
						cond.comparison = (statement.GetLength() > 0) ? SoundConditional::Comparison::IS_SET : SoundConditional::Comparison::ALWAYS;
					}
					else if (opStart < opStop)
					{
						cond.comparison = SoundConditional::Comparison::ALWAYS;

						const char ch1 = *opStart;
						const char ch2 = (opStop - opStart > 1) ? (*(opStart + 1)) : 0;
						if (ch1 == '!' && ch2 == '=')
							cond.comparison = SoundConditional::Comparison::NOT_EQUAL;
						else if (ch1 == '=')
						{
							if (ch2 == 0 || ch2 == '=')
								cond.comparison = SoundConditional::Comparison::EQUAL;
							else if (ch2 == '>')
								cond.comparison = SoundConditional::Comparison::GREATER_EQUAL;
							else if (ch2 == '<')
								cond.comparison = SoundConditional::Comparison::LESS_EQUAL;
						}
						else if (ch1 == '>')
						{
							if (ch2 == 0)
								cond.comparison = SoundConditional::Comparison::GREATER;
							else if (ch2 == '=')
								cond.comparison = SoundConditional::Comparison::GREATER_EQUAL;
						}
						else if (ch1 == '<')
						{
							if (ch2 == 0)
								cond.comparison = SoundConditional::Comparison::LESS;
							if (ch2 == '=')
								cond.comparison = SoundConditional::Comparison::LESS_EQUAL;
						}

						if (cond.comparison == SoundConditional::Comparison::ALWAYS)
						{
							ParseError("unknown operator");
							continue;
						}

						k = statement.SubString(0, opStart - statement.begin());
						v = statement.SubString(opStop - statement.begin());
					}
					else
					{
						ParseError("unexpected error while parsing conditional!");
						return false;
					}

					if (StringUtils::Equal_CaseInsensitive(k.begin(), "from")) cond.field = ESoundContextData::FROM;
					else if (StringUtils::Equal_CaseInsensitive(k.begin(), "count")) cond.field = ESoundContextData::COUNT;
					else if (StringUtils::Equal_CaseInsensitive(k.begin(), "lines")) cond.field = ESoundContextData::LINES;
					else
					{
						ParseError(CSTR("Unknown conditional param \"", k, '\"'));
						continue;
					}

					switch (cond.field)
					{
					case ESoundContextData::COUNT:
					case ESoundContextData::LINES:
						cond.mInt = v.ToInt();
						break;
					case ESoundContextData::FROM:
						cond.mString = v;
						break;
					}
				}
			}

			continue;
		}
		else if (*c == ']')
		{
			ParseError("\']\' encountered before suitable opening bracket");
			return false;
		}
		else if (*c == '{')
		{
			if (!bucket && argCount <= 0)
			{
				ParseError("Cannot create a nameless root-level bucket!");
				return false;
			}

			SoundBucket* parentBucket = bucket;
			bucket = bucketStack.Push(&buckets.EmplaceBack());

			if (parentBucket)
			{
				SoundBucket::Entry& entry = parentBucket->entries.Emplace();
				entry.bucket = bucket;
				entry.condition = cond;
			}

			cond.comparison = SoundConditional::Comparison::ALWAYS;

			for (int i = 0; i < argCount; ++i)
			{
				if (args[i][0] == '(')
				{
					if (i != argCount - 1)
					{
						ParseError("Only the last argument before a bucket can use rounded brackets!");
						return false;
					}

					args[i] = args[i].SubString(1, args[i].GetLength());
					Buffer<String> statements = args[i].Split(" \t\r\n; ");
					for (const String& statement : statements)
					{
						String k, v;
						
						const size_t firstEq = IteratorUtils::IndexOf(statement.begin(), statement.end(), '=');
						k = statement.SubString(0, firstEq);
						v = statement.SubString(firstEq + 1);

						if (StringUtils::Equal_CaseInsensitive(k.begin(), "pick"))
						{
							if (StringUtils::Equal_CaseInsensitive(v.begin(), "first")) bucket->picker.mode = ESoundBucketPickMode::FIRST;
							else if (StringUtils::Equal_CaseInsensitive(v.begin(), "seq")) bucket->picker.mode = ESoundBucketPickMode::SEQUENTIAL;
							else if (StringUtils::Equal_CaseInsensitive(v.begin(), "rand")) bucket->picker.mode = ESoundBucketPickMode::RANDOM;
							else if (StringUtils::Equal_CaseInsensitive(v.begin(), "shuffle")) bucket->picker.mode = ESoundBucketPickMode::SHUFFLE;
							else
							{
								ParseError(CSTR("unknown pick mode \"", v.begin(), '\"'));
								continue;
							}
						}
					}
				}
				else
				{
					if (parentBucket)
					{
						Debug::PrintLine("Sub-buckets may not be for a different event!");
						return false;
					}

					//This could be optimised
					for (size_t soundType = 0; soundType < (size_t)ESoundEvent::_COUNT; ++soundType)
						if (StringUtils::Equal_CaseInsensitive(args[i].begin(), g_soundTypeNames[soundType]))
						{
							sounds[soundType] = bucket;
							break;
						}
				}
			}

			argCount = 0;
		}
		else if (*c == '}')
		{
			if (!bucket)
			{
				ParseError("\'}\' cannot pop root scope!");
				return false;
			}

			SoundBucket** bucketPtrOrNull = bucketStack.Pop();
			bucket = bucketPtrOrNull ? *bucketPtrOrNull : nullptr;
		}
		else
		{
			const char* wordStart = c;

			while (true)
			{
				if (*c == '\0' || *c == ' ' || *c == '\t' || *c == '\r' || *c == '\n' || IsBracket(*c))
					break;

				++c;
			}

			args[argCount++] = String(wordStart, c - wordStart);

			if (*c == ';' || *c == '\r' || *c == '\n' || *c == '\0')
			{
				if (bucket)
				{
					for (int i = 0; i < argCount; ++i)
					{
						SoundBucket::Entry& entry = bucket->entries.Emplace();
						entry.sound = g_engine.sounds->Get(QualifySound(curDir, args[i]));
						entry.condition = cond;
					}
				}
				else if (argCount >= 2)
				{
					SharedPointer<SoundClip> soundRef = g_engine.sounds->Get(QualifySound(curDir, args[argCount - 1]));
					SoundBucket* addedBucket = soundRef ? &buckets.EmplaceBack() : nullptr;

					if (addedBucket)
					{
						SoundBucket::Entry& entry = addedBucket->entries.Emplace();
						entry.sound = soundRef;
						entry.condition = cond;
					}

					for (int i = 0; i < argCount - 1; ++i)
					{
						//This could be optimised
						for (size_t soundType = 0; soundType < (size_t)ESoundEvent::_COUNT; ++soundType)
							if (StringUtils::Equal_CaseInsensitive(args[i].begin(), g_soundTypeNames[soundType]))
							{
								addedBucket = sounds[soundType] = addedBucket;
								break;
							}
					}
				}

				cond.comparison = SoundConditional::Comparison::ALWAYS;
				argCount = 0;
			}

			continue;
		}

		++c;
	}

	return true;
}
