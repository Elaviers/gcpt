#pragma once
#include "string.h"
#include "sound_clip.h"
#include "shared/t_list.h"
#include "shared/shared_ptr.h"
#include "shared/random.h"

enum class ESoundEvent
{
	NONE = 0,

	//game
	START, LEVELUP, LEVELDN, WIN, LOSE,
	CLEAR, //tetris line clear, puyo pop
	ALLCLEAR,

	//tetris only
	TETRIS_BACK2BACK, TETRIS_TSPIN,
	
	//puyo only
	PUYO_COMBO,
	
	//piece movement
	LOCK,
	ROTLEFT, ROTRIGHT, ROTDOUBLE,
	MOVLEFT, MOVRIGHT,
	DROP, SOFTDROP, HARDDROP,

	//ui
	UI_NAV_UP, UI_NAV_DN,
	UI_SWITCH_TAB,
	UI_CONFIRM, UI_CANCEL,
	UI_PAUSE,

	//mus
	MUS_BG,
	MUS_DANGER,

	_COUNT
};

enum class ESoundDataType
{
	STRING, INT, FLOAT
};

enum class ESoundContextData
{
	FROM, COUNT, LINES,
	_COUNT
};

enum class ESoundBucketPickMode
{
	FIRST, SEQUENTIAL, RANDOM, SHUFFLE,
	_COUNT
};

static ESoundDataType soundContextDataTypes[(size_t)ESoundContextData::_COUNT] =
{
	ESoundDataType::STRING, ESoundDataType::INT, ESoundDataType::INT
};

struct SoundConditional
{
	enum class Comparison { ALWAYS, IS_SET, IS_NOT_SET, EQUAL, NOT_EQUAL, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL } comparison;
	ESoundContextData field;

	String mString;
	union
	{
		int mInt;
		float mFloat;
	};

	SoundConditional() : comparison(Comparison::ALWAYS), mInt(0) {}
	~SoundConditional() {}
};

extern const char* g_soundTypeNames[(size_t)ESoundEvent::_COUNT];

class SoundBucket
{
public:
	struct Entry
	{
		SoundConditional condition;
		SharedPointer<SoundClip> sound;
		SoundBucket* bucket;

		Entry() : bucket(nullptr) {}
	};

	struct Picker
	{
		ESoundBucketPickMode mode;
		int counter;
	};

	Picker picker;
	Buffer<Entry> entries;
};

class SoundConfig
{
public:
	String context;
	List<SoundBucket> buckets;
	SoundBucket* sounds[(size_t)ESoundEvent::_COUNT];

	bool Parse(const String& name, const char* string);
};

struct SoundParams
{
	int count;
	int lines;
};

extern SoundConfig* g_currentSoundConfig;

void SoundEngine_PlayEvent(ESoundEvent sound, const SoundParams* params, Random& rand);
