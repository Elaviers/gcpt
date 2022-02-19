#include "puyoboard.h"

void PuyoBoard::Setup()
{

}

void PuyoBoard::BeginPlay()
{

}

void PuyoBoard::Frame(float lastFrameTime)
{



}

#ifdef BUILD_CLIENT
#include "client/render_entry.h"
#include "client/render_queue.h"

void PuyoBoard::Render(RenderQueue& queue) const
{

	RenderEntry& puyos = queue.CreateEntry(ERenderChannels::BOARD);
	for (size_t x = 0; x < elements.GetWidth(); ++x)
		for (size_t y = 0; y < elements.GetHeight(); ++y)
		{
			const PuyoElement& element = elements.At(x, y);
			puyos.AddSetTextureGL(element.textureId, 0);

		}

}

#endif // BUILD_CLIENT
