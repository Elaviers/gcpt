#pragma once
#include "board.h"
#include "bag.h"
#include "t_matrix.h"

#ifdef BUILD_CLIENT

#endif // BUILD_CLIENT

class PuyoElement
{
public:
	uint16 textureId;
};

class PuyoBoard : public Board
{
public:
	Matrix<PuyoElement> elements;
	Matrix<PuyoElement> activePiece;
	Bag<byte> bag;

	float _lastMoveLeft;
	float _lastMoveRight;
	float _lastDrop;

	void Setup();
	void BeginPlay();

	virtual void Frame(float lastFrameTime) override;

#ifdef BUILD_CLIENT
	virtual void Render(RenderQueue&) const override;
#endif // BUILD_CLIENT
};
