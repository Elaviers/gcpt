#include "ui_rect.h"
#include "render_cmd.h"
#include "render_queue.h"

void UIRect::UpdateBounds()
{
	UIElement::UpdateBounds();
	_transform.SetPosition(Vector3(_absoluteBounds.x + _absoluteBounds.w / 2.f, _absoluteBounds.y + _absoluteBounds.h / 2.f, _z));
	_transform.SetScale(Vector3(_absoluteBounds.w, _absoluteBounds.h, 0.f));
}

void UIRect::Render(RenderQueue& q) const
{
	RenderEntry& e = q.CreateEntry(ERENDERCHANNEL_ELUI, 0);
	_colour.Apply(e);

	if (_texture)
		e.AddSetTexture(*_texture, 0);
	else
		e.AddSetTexture(RCMDSetTexture::Type::WHITE, 0);
	
	e.AddSetTransform(_transform.GetMatrix());
	e.AddCommand(RCMDSetUVOffset::Default());
	e.AddCommand(RCMDSetUVScale::Default());
	e.AddCommand(RCMDRenderMesh::PLANE);
}
