#pragma once
#include "board.h"
#include "t_matrix.h"
#include "t_vec2.h"

using Tetris_VidType = byte;

class TetrisElement
{
public:
	Tetris_VidType vid;
};

class TetrisBoard : public Board
{
public:
	Matrix<TetrisElement> elements;

	virtual void Frame(float lastFrameTime) override;

#ifdef BUILD_CLIENT
	virtual void Render(RenderQueue&) const override;
#endif // BUILD_CLIENT

};
