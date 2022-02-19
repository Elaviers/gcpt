#include "sound.h"
#include "shared/t_vec.h"
#include "shared/debug.h"

SoundConfig* g_currentSoundConfig;

template<typename T> bool _SoundComparison(const T& a, SoundConditional::Comparison cond, const T& b)
{
	switch (cond)
	{
	case SoundConditional::Comparison::EQUAL: return a == b;
	case SoundConditional::Comparison::NOT_EQUAL: return a != b;
	case SoundConditional::Comparison::GREATER: return a > b;
	case SoundConditional::Comparison::GREATER_EQUAL: return a >= b;
	case SoundConditional::Comparison::LESS: return a < b;
	case SoundConditional::Comparison::LESS_EQUAL: return a <= b;
	case SoundConditional::Comparison::ALWAYS: return true;
	}

	return false;
}

static bool SoundEngine_ConditionMet(const SoundConditional& cond, ESoundEvent lastSound, const SoundParams* params)
{
	if (cond.comparison == SoundConditional::Comparison::ALWAYS)
		return true;

	float floatVal = 0.f;
	int intVal = 0;
	const char* strVal = "";
	bool isSet = false;
	enum { T_INT, T_FLOAT, T_STRING } typeSet = T_INT;

	switch (cond.field)
	{
	case ESoundContextData::FROM:
		strVal = g_soundTypeNames[(int)lastSound];
		isSet = strVal[0];
		typeSet = T_STRING;
		break;

	case ESoundContextData::COUNT:
		if (params)
		{
			intVal = params->count;
			isSet = intVal != 0;
			typeSet = T_INT;
		}
		break;

	case ESoundContextData::LINES:
		if (params)
		{
			intVal = params->lines;
			isSet = intVal != 0;
			typeSet = T_INT;
		}
		break;

	default:
		Debug::PrintLine("Error: encountered field with invalid condition field", "[Sound]");
	}

	switch (cond.comparison)
	{
	case SoundConditional::Comparison::IS_SET:
		return isSet;

	case SoundConditional::Comparison::IS_NOT_SET:
		return !isSet;
	
	default:
		switch (typeSet)
		{
		case T_INT:
			return _SoundComparison<int>(intVal, cond.comparison, cond.mInt);
		case T_FLOAT:
			return _SoundComparison<float>(floatVal, cond.comparison, cond.mFloat);
		case T_STRING:
			return _SoundComparison<String>(strVal, cond.comparison, cond.mString);
		default:
			Debug::PrintLine("Error: typeSet is invalid", "[Sound]");
		}
	}

	return false;
}

void SoundEngine_PlayEvent(ESoundEvent sound, const SoundParams* params, Random& rand)
{

	if (g_currentSoundConfig)
	{
		SoundBucket* const bucket = g_currentSoundConfig->sounds[(int)sound];
		
		if (bucket)
		{
			static TVecStaticGrowable<int, 64> picks;
			picks.Reserve(bucket->entries.GetSize());

			for (size_t i = 0; i < picks.GetSize(); ++i)
				picks.Add(i);


			int pickIdx = 0;
			ESoundBucketPickMode pickMode = bucket->picker.mode;

			if (pickMode == ESoundBucketPickMode::RANDOM)
				for (size_t i = 0; i < picks.GetSize(); ++i)
					Utilities::Swap(picks[i], picks[rand.Next(picks.GetSize())]);

			if (pickMode == ESoundBucketPickMode::RANDOM)
			{
				pickIdx = rand.Next(bucket->entries.GetSize());



			}

			if (bucket->picker.mode == ESoundBucketPickMode::SHUFFLE && bucket->picker.counter >= bucket->entries.GetSize())
			{
				

			}

		}
	}
}
