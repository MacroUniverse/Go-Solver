#pragma once
#include "config.h"
#include "board.h"

// constant reference type to board
// has all const functions of Board
// cannot be edited
class BoardRef
{
private:
	const Config & m_config;
	Trans m_trans; // trans needed from m_config to actual board

public:
	// === constructors ===
	BoardRef(Config_I &config, Trans_I trans) : m_config(config), m_trans(trans) {}

	// === const functions ===

	// get a stone by coordinate
	Who operator()(Char_I x, Char_I y) const
	{
		return m_config.transform1(x, y, m_trans);
	}

	// display config on screen
	void disp() const;

	const Config & config() const
	{
		return m_config;
	}

	Trans trans() const
	{
		return m_trans;
	}

	// check if a placing is legal, or how many stones will be dead
	// same check already exists for place()
	// Ko is not considered!
	// if legal, return the number of stones that can be removed ( >= 0)
	// return -1 if occupied, do nothing
	// return -2 if no qi, do nothing
	Int check(Char_I x, Char_I y, Who_I who) const
	{
		Char x1 = x, y1 = y;
		Who who1 = who;
		inv_transf(x1, y1, who1, m_trans);
		return m_config.check(x1, y1, who1);
	}

	// if the game has ended
	// game only ends when only eyes and mutual life (two qi's) are left
	Bool is_game_end() const
	{
		return m_config.is_game_end();
	}

	// calculate territory
	// this is only accurate when only dumb eye filling exists
	// X's territory = # X's stone on board + # single qi's surrounded by X + (other qi's not surrounded by Y)/2
	Int calc_territory2(Who_I who) const
	{
		if (m_trans.flip())
			return m_config.calc_territory2(next(who));
		return m_config.calc_territory2(who);
	}

	// is (x,y) an eye surrounded by who
	Bool is_eye(Char_I x, Char_I y, Who_I who) const
	{
		Char x1 = x, y1 = y;
		Who who1 = who;
		inv_transf(x1, y1, who1, m_trans);
		return m_config.is_eye(x1, y1, who1);
	}

	// if filling of an eye that's not in danger
	Bool is_dumb_eye_filling(Char_I x, Char_I y, Who_I who) const
	{
		Char x1 = x, y1 = y;
		Who who1 = who;
		inv_transf(x1, y1, who1, m_trans);
		return m_config.is_dumb_eye_filling(x1, y1, who1);
	}

	// if filling a big eye surrounded by connected stones
	Bool is_dumb_2eye_filling(Char_I x, Char_I y, Who_I who) const
	{
		Char x1 = x, y1 = y;
		Who who1 = who;
		inv_transf(x1, y1, who1, m_trans);
		return m_config.is_dumb_2eye_filling(x1, y1, who1);
	}
};

typedef const BoardRef BoardRef_I;

inline void BoardRef::disp() const
{
	Char i, x, y, Nx = board_Nx(), Ny = board_Ny();
	cout << "     ";
	for (i = 0; i < board_Nx(); ++i) cout << Int(i) << "   "; cout << "\n";
	cout << "    ";
	for (x = 0; x < Nx; ++x) cout << "----"; cout << "\n";

	for (y = 0; y < Ny; ++y) {
		cout << " " << Int(y) << " |";
		for (x = 0; x < Nx; ++x) {
			if (operator()(x, y) == Who::NONE)
				cout << "   |";
			else if (operator()(x, y) == Who::BLACK)
				cout << " @ |";
			else if (operator()(x, y) == Who::WHITE)
				cout << " O |";
			else
				error("Config::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "    ";
		for (i = 1; i <= Nx; ++i) cout << "----"; cout << "\n";
	}
}
