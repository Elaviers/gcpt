#include "material_grid.h"
#include "material_param.h"
#include "render_cmd.h"
#include "render_entry.h"
#include "texture_manager.h"
#include "shared/macro_utils.h"
#include "shared/engine.h"

const PropertyCollection& MaterialGrid::GetProperties()
{
	static PropertyCollection properties;

	IF_FIRST_EXEC_DO({
		properties.AddCommand("texture", MemberCommandPtr<MaterialGrid>(&MaterialGrid::_CMD_texture));
		properties.AddCommand("rows", MemberCommandPtr<MaterialGrid>(&MaterialGrid::_CMD_rows));
		properties.AddCommand("columns", MemberCommandPtr<MaterialGrid>(&MaterialGrid::_CMD_columns));
		properties.AddCommand("element", MemberCommandPtr<MaterialGrid>(&MaterialGrid::_CMD_element));
	});

	return properties;
}

void MaterialGrid::_CMD_texture(const Array<String> &args)
{
	if (args.GetSize() > 0)
	{
		TextureManager* textureManager = g_engine.textures;
		if (textureManager)
		{
			_texture = textureManager->Get(args[0]);
			_TryCalculateElements();
		}
	}
}

void MaterialGrid::_CMD_rows(const Array<String> &args)
{
	_rowHeights.Clear();

	for (size_t i = 0; i < args.GetSize(); ++i)
		_rowHeights.Add(args[i].ToInt());

	_TryCalculateElements();
}

void MaterialGrid::_CMD_columns(const Array<String> &args)
{
	_columnWidths.Clear();

	for (size_t i = 0; i < args.GetSize(); ++i)
		_columnWidths.Add(args[i].ToInt());

	_TryCalculateElements();
}

void MaterialGrid::_CMD_element(const Array<String>& args)
{
	if (args.GetSize() >= 5)
	{
		int r = args[0].ToInt();
		int c = args[1].ToInt();

		GridElement& element = GetElement(r, c);

		element.textureOverride = g_engine.textures->Get(args[2]);
		element.scale.x = args[3].ToFloat();
		element.scale.y = args[4].ToFloat();
		element.offset.x = element.offset.y = 0.f;
	}
}

void MaterialGrid::_TryCalculateElements()
{
	size_t rows = _rowHeights.GetSize();
	size_t columns = _columnWidths.GetSize();

	if (rows && columns && _texture->IsValid())
	{
		_elements.SetSize(rows * columns);

		float y = 0;
		for (size_t r = 0; r < rows; ++r)
		{
			float h = (float)_rowHeights[r] / (float)_texture->GetHeight();

			float x = 0;
			for (size_t c = 0; c < columns; ++c)
			{
				size_t index = r * columns + c;
				float w = (float)_columnWidths[c] / (float)_texture->GetWidth();

				if (!_elements[index].textureOverride)
				{
					_elements[index].offset = Vector2(x, y);
					_elements[index].scale = Vector2(w, h);
				}

				x += w;
			}

			y += h;
		}
	}
}

void MaterialGrid::Apply(RenderEntry& e, const MaterialParam *param) const
{
	if (param && param->type == MaterialParam::EType::GRID_PARAM)
	{
		const GridElement& element = GetElement(param->gridData.row, param->gridData.column);

		if (element.textureOverride)
		{
			e.AddSetTexture(*element.textureOverride, 0);
			e.AddSetUVScale(param->gridData.uvScale * element.scale);
		}
		else if (_texture)
		{
			e.AddSetTexture(*_texture, 0);
			e.AddSetUVScale(element.scale); //gotta stretch, can't use parent uvscale
		}
		else
		{
			e.AddSetTexture(RCMDSetTexture::Type::BLACK, 0);
			e.AddSetUVScale();
		}
		
		e.AddSetUVOffset(element.offset);
	}
}
