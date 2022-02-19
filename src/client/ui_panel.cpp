#include "ui_panel.h"
#include "shared/exceptions.h"
#include "material_grid.h"
#include "material_param.h"
#include "render_cmd.h"
#include "render_queue.h"

void UIPanel::Render(RenderQueue& q) const
{
	RenderEntry& e = q.CreateEntry(ERENDERCHANNEL_ELUI);
	_colour.Apply(e);

	if (!_material)
	{
		e.AddSetUVScale();
		e.AddSetUVOffset();
		e.AddSetTexture(RCMDSetTexture::Type::WHITE, 0);
		e.AddSetTransform(_transforms[4].GetMatrix());
		e.AddCommand(RCMDRenderMesh::PLANE);
		return;
	}

	const MaterialGrid* grid = dynamic_cast<const MaterialGrid*>(_material.Ptr());
	if (grid)
	{
		MaterialParam param;
		param.type = MaterialParam::EType::GRID_PARAM;

		int rc = grid->GetRowCount();
		int cc = grid->GetColumnCount();

		if (cc >= 3 && rc == 1)
		{
			float h = _transforms[0].GetScale().y + _transforms[3].GetScale().y + _transforms[6].GetScale().y;

			param.gridData.uvScale = Vector2(1.f, 1.f);
			param.gridData.row = param.gridData.column = 0;
			_material->Apply(e, &param);
			Transform t = _transforms[3];
			t.SetScale(Vector3(t.GetScale().x, h, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.column = 2;
			_material->Apply(e, &param);
			t = _transforms[5];
			t.SetScale(Vector3(t.GetScale().x, h, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.uvScale = Vector2(_uvScaling.x, 1.f);
			param.gridData.column = 1;
			_material->Apply(e, &param);
			t = _transforms[4];
			t.SetScale(Vector3(t.GetScale().x, h, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);
		}
		else if (rc >= 3 && cc == 1)
		{
			float w = _transforms[0].GetScale().x + _transforms[1].GetScale().x + _transforms[2].GetScale().x;

			param.gridData.uvScale = Vector2(1.f, 1.f);
			param.gridData.row = param.gridData.column = 0;
			_material->Apply(e, &param);
			Transform t = _transforms[1];
			t.SetScale(Vector3(w, t.GetScale().y, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.row = 2;
			_material->Apply(e, &param);
			t = _transforms[7];
			t.SetScale(Vector3(w, t.GetScale().y, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.uvScale = Vector2(1.f, _uvScaling.y);
			param.gridData.row = 1;
			_material->Apply(e, &param);
			t = _transforms[4];
			t.SetScale(Vector3(w, t.GetScale().y, t.GetScale().z));
			e.AddSetTransform(t.GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);
		}
		
		else if (rc >= 3 && cc >= 3)
		{
			//Full bordered panel
			//centre
			param.gridData.uvScale = Vector2(1.f, 1.f);
			param.gridData.row = param.gridData.column = 1;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[4].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			//Corners
			param.gridData.row = param.gridData.column = 0;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[0].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.column = 2;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[2].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.row = 2;
			param.gridData.column = 0;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[6].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.column = 2;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[8].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			//X edges
			param.gridData.uvScale = Vector2(_uvScaling.x, 1.f);

			param.gridData.row = 0;
			param.gridData.column = 1;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[1].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.row = 2;
			param.gridData.column = 1;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[7].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			//Y edges
			param.gridData.uvScale = Vector2(1.f, _uvScaling.y);

			param.gridData.row = 1;
			param.gridData.column = 0;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[3].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);

			param.gridData.column = 2;
			_material->Apply(e, &param);
			e.AddSetTransform(_transforms[5].GetMatrix());
			e.AddCommand(RCMDRenderMesh::PLANE);
		}
		else
		{
			Debug::Message(CSTR("Invalid row/column grid material setup for UIPanel (", rc, " rows, ", cc, " columns)"), "Go and fix this");
		}
	}
	else
	{
		e.AddSetUVScale();
		e.AddSetUVOffset();
		_material->Apply(e);
		e.AddSetTransform(_transforms[4].GetMatrix());
		e.AddCommand(RCMDRenderMesh::PLANE);
	}
}

void UIPanel::UpdateBounds()
{
	UIElement::UpdateBounds();

	const float& topBorderSz = _borderSizes[0];
	const float& rightBorderSz = _borderSizes[1];
	const float& bottomBorderSz = _borderSizes[2];
	const float& leftBorderSz = _borderSizes[3];
	float absTopBorderSz = Maths::Abs(topBorderSz);
	float absRightBorderSz = Maths::Abs(rightBorderSz);
	float absBottomBorderSz = Maths::Abs(bottomBorderSz);
	float absLeftBorderSz = Maths::Abs(leftBorderSz);

	float x0 = _absoluteBounds.x + leftBorderSz / 2.f;
	float y0 = _absoluteBounds.y + bottomBorderSz / 2.f;
	float x2 = _absoluteBounds.x + _absoluteBounds.w - rightBorderSz / 2.f;
	float y2 = _absoluteBounds.y + _absoluteBounds.h - topBorderSz / 2.f;

	float centreX = ((x0 + absLeftBorderSz / 2.f) + (x2 - absRightBorderSz / 2.f)) / 2.f;
	float centreY = ((y0 + absBottomBorderSz / 2.f) + (y2 - absTopBorderSz / 2.f)) / 2.f;
	float centreW = _absoluteBounds.w - rightBorderSz - leftBorderSz;
	float centreH = _absoluteBounds.h - topBorderSz - bottomBorderSz ;
	
	_transforms[0].SetScale(Vector3(absLeftBorderSz, absTopBorderSz, 1.f));
	_transforms[2].SetScale(Vector3(absRightBorderSz, absTopBorderSz, 1.f));
	_transforms[6].SetScale(Vector3(absLeftBorderSz, absBottomBorderSz, 1.f));
	_transforms[8].SetScale(Vector3(absRightBorderSz, absBottomBorderSz, 1.f));
	_transforms[1].SetScale(Vector3(centreW, absTopBorderSz, 1.f));
	_transforms[7].SetScale(Vector3(centreW, absBottomBorderSz, 1.f));
	_transforms[3].SetScale(Vector3(absLeftBorderSz, centreH, 1.f));
	_transforms[5].SetScale(Vector3(absRightBorderSz, centreH, 1.f));
	_transforms[4].SetScale(Vector3(centreW, centreH, 1.f));

	_transforms[0].SetPosition(Vector3(x0,		y2,			_z));
	_transforms[1].SetPosition(Vector3(centreX, y2,			_z));
	_transforms[2].SetPosition(Vector3(x2,		y2,			_z));
	_transforms[3].SetPosition(Vector3(x0,		centreY,	_z));
	_transforms[4].SetPosition(Vector3(centreX, centreY,	_z));
	_transforms[5].SetPosition(Vector3(x2,		centreY,	_z));
	_transforms[6].SetPosition(Vector3(x0,		y0,			_z));
	_transforms[7].SetPosition(Vector3(centreX, y0,			_z));
	_transforms[8].SetPosition(Vector3(x2,		y0,			_z));

	_uvScaling.x = centreW / ((absLeftBorderSz + absRightBorderSz) / 2.f);
	_uvScaling.y = centreH / ((absBottomBorderSz + absTopBorderSz) / 2.f);
}

float UIPanel::GetBorderSize(EBorder border) const
{
	switch (border)
	{
	case UIPanel::EBorder::TOP: return _borderSizes[0];
	case UIPanel::EBorder::RIGHT: return _borderSizes[1];
	case UIPanel::EBorder::BOTTOM: return _borderSizes[2];
	case UIPanel::EBorder::LEFT: return _borderSizes[3];
	}

	throw ExcInvalidParam("Border must be TOP,RIGHT,BOTTOM, or LEFT. Bitfield not allowed here.");
}

UIPanel& UIPanel::SetBorderSize(EBorder borders, float size)
{
	if ((int)(borders & EBorder::TOP)) _borderSizes[0] = size;
	if ((int)(borders & EBorder::RIGHT)) _borderSizes[1] = size;
	if ((int)(borders & EBorder::BOTTOM)) _borderSizes[2] = size;
	if ((int)(borders & EBorder::LEFT)) _borderSizes[3] = size;

	return *this;
}
