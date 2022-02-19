#pragma once
#include "shared/colour.h"

class RenderEntry;

class UIColour
{
	bool _isBlend;

	union
	{
		Colour _colour;
		Colour _primary;
	};

	Colour _secondary;
	Colour _tertiary;

	friend class UIColourModifier;

public:
	UIColour(const Colour& colour = Colour::White) : _colour(colour), _isBlend(false) {}
	UIColour(const Colour& primary, const Colour& secondary) : _primary(primary), _secondary(secondary), _tertiary(primary), _isBlend(true) {}
	UIColour(const Colour& primary, const Colour& secondary, const Colour& tertiary) : _primary(primary), _secondary(secondary), _tertiary(tertiary), _isBlend(true) {}
	~UIColour() {}

	UIColour(const UIColour& other) : _isBlend(other._isBlend), _primary(other._primary), _secondary(other._secondary), _tertiary(other._tertiary) {}

	UIColour& operator=(const UIColour& other)
	{
		_isBlend = other._isBlend;
		_primary = other._primary;
		_secondary = other._secondary;
		_tertiary = other._tertiary;
		
		return *this;
	}

	void Apply(RenderEntry&) const;
};

enum class EColourModifyMode
{
	NOP = 0,
	SET,
	ADD,
	MUL
};

class UIColourModifier
{
	EColourModifyMode _primaryModifier;
	EColourModifyMode _secondaryModifier;
	EColourModifyMode _tertiaryModifier;

	Colour _primary;
	Colour _secondary;
	Colour _tertiary;

public:
	UIColourModifier() : _primaryModifier(EColourModifyMode::NOP), _secondaryModifier(EColourModifyMode::NOP), _tertiaryModifier(EColourModifyMode::NOP) {}
	UIColourModifier(EColourModifyMode mode, const UIColour& colour) : _primary(colour._primary), _secondary(colour._secondary), _tertiary(colour._tertiary), _primaryModifier(mode), _secondaryModifier(mode), _tertiaryModifier(mode) {}

	void SetPrimary(EColourModifyMode mode, Colour primary) { _primaryModifier = mode; _primary = primary; }
	void SetSecondary(EColourModifyMode mode, Colour primary) { _secondaryModifier = mode; _secondary = primary; }
	void SetTertiary(EColourModifyMode mode, Colour primary) { _tertiaryModifier = mode; _tertiary = primary; }

	void ModifyColour(UIColour& colour) const;
};
