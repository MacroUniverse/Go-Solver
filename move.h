#pragma once
#include "common.h"

class Move;
typedef const Move Move_I;
typedef Move Move_O, Move_IO;

// specify a move
class Move
{
private:
	// === data members ===

	// coordinates of the board
	// special codes:
	// m_x = -1: pass
	// m_x = -2: edit board
	// m_x = -3: game init (for initial node 0 with empty board)
	Char m_x, m_y; // coordinates or special codes

public:
	// === constructors ===
	Move() {}
	Move(const Move &mov) { m_x = mov.m_x; m_y = mov.m_y; }
	Move(Char_I x, Char_I y);
	Move(Act_I act);

	// === const functions ===

	Act type() const; // move type

	Bool isplace() const; // if is a PLACE

	Bool ispass() const; // if is a PASS

	Bool isedit() const; // if is an EDIT

	Bool isinit() const; // if is an INIT

	Char x() const; // x coordinate of PLACE

	Char y() const; // y coordinate of PLACE

	// === none-const functions ===
	
	void place(Char_I x, Char_I y); // place a stone

	void pass() { m_x = -1; } // pass

	void edit() { m_x = -2; } // edit board

	void init() { m_x = -3; } // init game

	// === destructor ===
	~Move() {}
};

inline Move::Move(Char_I x, Char_I y) : m_x(x), m_y(y)
{
#ifdef GOS_CHECK_BOUND
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Move::Move(x, y): coord < 0 !");
#endif
}

inline Move::Move(Act_I act)
{
	if (act == Act::PASS)
		m_x = -1;
	else if (act == Act::EDIT)
		m_x = -2;
	else if (act == Act::INIT)
		m_x = -3;
	else
		error("illegal Act type!");
}

inline Act Move::type() const
{
	return isplace() ? Act::PLACE : ispass() ? Act::PASS : isedit() ? Act::EDIT : Act::INIT;
}

inline Bool Move::isplace() const
{
	return m_x >= 0;
}

inline Bool Move::ispass() const
{
	return m_x == -1;
}

inline Bool Move::isedit() const
{
	return m_x == -2;
}

inline Bool Move::isinit() const
{
	return m_x == -3;
}

inline Char Move::x() const
{
#ifdef GOS_CHECK_BOUND
	if (m_x < 0)
		error("Move::x(): not a coord!");
#endif
	return m_x;
}

inline Char Move::y() const
{
#ifdef GOS_CHECK_BOUND
	if (m_x < 0) error("Move::y(): not a coord!");
#endif
	return m_y;
}

inline void Move::place(Char_I x, Char_I y)
{
#ifdef GOS_CHECK_BOUND
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Move::Move(x, y): coord < 0 !");
#endif
	m_x = x; m_y = y;
}

Bool operator==(Move_I &mov1, Move_I &mov2)
{
	if (mov1.type() == mov2.type()) {
		if (mov1.isplace()) {
			return mov1.x() == mov2.x() && mov1.y() == mov2.y();
		}
		return true;
	}
	return false;
}
