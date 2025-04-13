#include "board.h"

Board::Board(int width, int height) : _boardWidth(width), _boardHeight(height)
{
	_board = new byte[width * height](); // Should zero it out 
}

Board::~Board() 
{
	delete[] _board;

}

void Board::Frame(float lastFrameTime)
{
	
}

void Board::Render(RenderQueue& queue) const
{
	// Render the board using the provided render queue
	// This is where you would add your rendering logic
}

