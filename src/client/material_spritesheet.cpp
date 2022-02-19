#include "material_spritesheet.h"
#include "material_param.h"

void MaterialSpritesheet::_UpdateFrameData()
{
	if (_diffuse)
	{
		_frameUV.x = _frameSize.x / _diffuse->GetWidth();
		_frameUV.y = _frameSize.x / _diffuse->GetWidth();
		_framesPerRow = (int)(_diffuse->GetWidth() / _frameSize.x);
	}
	else
	{
		_frameUV.x = _frameUV.y = 1.f;
		_framesPerRow = 1;
	}
}

const PropertyCollection& MaterialSpritesheet::GetProperties()
{
	static PropertyCollection properties;
	IF_FIRST_EXEC_DO({
		properties.AddCommand("tex", MemberCommandPtr<MaterialSpritesheet>(&MaterialSpritesheet::_CMD_tex));
		properties.Add("framesize",
			MemberGetter<MaterialSpritesheet, const Vector2&>(&MaterialSpritesheet::_GetFrameSize),
			MemberSetter<MaterialSpritesheet, Vector2>(&MaterialSpritesheet::_SetFrameSize));
		properties.Add<int>("framecount", offsetof(MaterialSpritesheet, _frameCount));
		properties.Add<float>("framerate", offsetof(MaterialSpritesheet, _frameRate));
	});

	return properties;
}

void MaterialSpritesheet::Apply(RenderEntry& e, const MaterialParam* param) const
{
	if (!_diffuse)
	{
		e.AddSetTexture(RCMDSetTexture::Type::WHITE, 0);
		return;
	}

	e.AddSetTexture(*_diffuse, 0);

	e.AddSetUVScale(_frameUV);
	
	if (param && param->type == MaterialParam::EType::SPRITESHEET_PARAM)
	{
		int frame = (int)(param->spritesheetData.time * _frameRate) % _frameCount;
		int r = frame / _framesPerRow;
		int c = frame % _framesPerRow;

		e.AddSetUVOffset(Vector2(_frameUV.x * c, _frameUV.y * r));
	}
	else
	{
		e.AddSetUVOffset();
	}
}
