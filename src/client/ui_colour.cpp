#include "ui_colour.h"
#include "render_cmd.h"
#include "render_entry.h"

void UIColour::Apply(RenderEntry& e) const
{
	if (_isBlend)
		e.AddSetColour(_primary, _secondary, _tertiary);
	else
		e.AddSetColour(_colour);
}

static void ModifyColourComponent(Colour& colour, EColourModifyMode mode, const Colour& op)
{
	switch (mode)
	{
	case EColourModifyMode::NOP:
		break;
	case EColourModifyMode::SET:
		colour = op;
		break;
	case EColourModifyMode::ADD:
		colour.r += op.r;
		colour.g += op.g;
		colour.b += op.b;
		colour.a += op.a;
		break;
	case EColourModifyMode::MUL:
		colour.r *= op.r;
		colour.g *= op.g;
		colour.b *= op.b;
		colour.a *= op.a;
		break;
	default:
		Assert(false); //Unsupported EColourModifyMode
		break;
	}
}

void UIColourModifier::ModifyColour(UIColour& colour) const
{
	ModifyColourComponent(colour._primary, _primaryModifier, _primary);
	ModifyColourComponent(colour._secondary, _secondaryModifier, _secondary);
	ModifyColourComponent(colour._tertiary, _tertiaryModifier, _tertiary);
}
