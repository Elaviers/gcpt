#pragma once
#include "material.h"
#include "texture.h"
#include "shared/t_buf.h"
#include "shared/shared_ptr.h"

class MaterialGrid : public Material
{
	struct GridElement
	{
		Vector2 offset;
		Vector2 scale;

		SharedPointer<const Texture> textureOverride;
	};

	SharedPointer<const Texture> _texture;

	Buffer<int> _rowHeights;
	Buffer<int> _columnWidths;

	Buffer<GridElement> _elements;

	void _TryCalculateElements();

	void _CMD_texture(const Array<String> &args);
	void _CMD_rows(const Array<String> &args);
	void _CMD_columns(const Array<String> &args);
	void _CMD_element(const Array<String> &args);

public:
	MaterialGrid() : Material(ERENDERCHANNEL_MATERIALGRID) {}

	virtual ~MaterialGrid() {}

	int GetRowCount() const { return (int)_rowHeights.GetSize(); }
	int GetColumnCount() const { return (int)_columnWidths.GetSize(); }

	const SharedPointer<const Texture>& GetTexture() const { return _texture; }
	void SetDiffuse(const SharedPointer<const Texture>& texture) { _texture = texture; }

	virtual const PropertyCollection& GetProperties() override;

	virtual void Apply(RenderEntry&, const MaterialParam *param) const override;

	virtual bool SetDefaultTexture(const SharedPointer<const Texture>& texture) override { SetDiffuse(texture); return true; }
	virtual const SharedPointer<const Texture>& GetDefaultTexture() const override { return _texture; }

	const GridElement& GetElement(int r, int c) const { return _elements[r * _columnWidths.GetSize() + c]; }
	GridElement& GetElement(int r, int c) { return _elements[r * _columnWidths.GetSize() + c]; }
};
