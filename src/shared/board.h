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

struct TetrisPiece
{
	int shapeWidth;
	int shapeHeight;

	bool shape[64][64];

	int x;
	int y;
};

class Board
{
	private:
		BoardInputState _inputState;
		TetrisPiece _tetrisPiece;
		byte* _board;
		int _boardWidth;
		int _boardHeight;

	public:
		Board(int width, int height);

		~Board();

		void Frame(float lastFrameTime);

#ifdef BUILD_CLIENT
		Transform renderTransform;

		void Render(RenderQueue& queue) const;
#endif // BUILD_CLIENT

		constexpr void SetInputState(uint16 inputState) { _inputState.all = inputState; }
};
