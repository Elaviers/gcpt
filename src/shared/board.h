#pragma once
#include "transform.h"

class RenderQueue;

struct BoardInputState
{
	union
	{
		struct
		{
			bool left : 1;
			bool right : 1;
			bool rotateLeft : 1;
			bool rotateRight : 1;
			bool hardDrop : 1;
			bool softDrop : 1;
			bool swap : 1;
		};

		uint16 all;
	};
};

class Board
{
protected:
	BoardInputState _inputState;

public:
	virtual void Frame(float lastFrameTime) = 0;

#ifdef BUILD_CLIENT
	Transform renderTransform;

	virtual void Render(RenderQueue& queue) const = 0;
#endif // BUILD_CLIENT

	constexpr void SetInputState(uint16 inputState) { _inputState.all = inputState; }
};
