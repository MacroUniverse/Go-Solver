#pragma once
#include "common.h"

// coordinates for a move, or "pass"
// origin at top left, x axis points right, y axis points down
// special codes:
// m_x = -1: pass
// m_x = -2: edit board
// m_x = -3: game init (for initial node 0 with empty board)
class Move
{
private:
	Char m_x, m_y; // coordinates or special codes
public:
	// constructor
	inline Move() {}
	inline Move(const Move &mov) { m_x = mov.m_x; m_y = mov.m_y; }
	inline Move(Char_I x, Char_I y);
	inline Move(Act_I act);

	// properties
	Bool isplace() const { return m_x >= 0; }
	Bool ispass() const { return m_x == -1; }
	Bool isedit() const { return m_x == -2; }
	Bool isinit() const { return m_x == -3; }
	Act type() // 0: isplace(), 1: ispass(), 2: isedit(), 3: isinit()
	{
		return isplace() ? Act::PLACE : ispass() ? Act::PASS : isedit() ? Act::EDIT : Act::INIT;
	}
	inline Char x() const;
	inline Char y() const;
	Bool operator==(const Move &rhs) const;

	// edit
	inline void place(Char_I x, Char_I y);
	void pass() { m_x = -1; }
	void edit() { m_x = -2; } // edit board
	void init() { m_x = -3; } // init game
	~Move() {}
};

inline Move::Move(Char_I x, Char_I y) : m_x(x), m_y(y)
{
#ifdef _CHECK_BOUND_
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

inline Char Move::x() const
{
#ifdef _CHECK_BOUND_
	if (m_x < 0) error("Move::x(): not a coord!");
#endif
	return m_x;
}

inline Char Move::y() const
{
#ifdef _CHECK_BOUND_
	if (m_x < 0) error("Move::y(): not a coord!");
#endif
	return m_y;
}

Bool Move::operator==(const Move &rhs) const
{
	if (m_x < 0)
		return m_x == rhs.m_x;
	else
		return m_x == rhs.m_x && m_y == rhs.m_y;
}

inline void Move::place(Char_I x, Char_I y)
{
#ifdef _CHECK_BOUND_
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Move::Move(x, y): coord < 0 !");
#endif
	m_x = x; m_y = y;
}
