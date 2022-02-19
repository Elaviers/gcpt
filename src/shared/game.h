#pragma once
#include "server.h"
#include "board.h"
#include "player.h"

class Game
{
private:
	Buffer<Player*> _players;
	Buffer<Board*> _boards;

public:
	bool Initialise();
	void Shutdown();


};
