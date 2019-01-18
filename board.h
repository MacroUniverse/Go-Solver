#pragma once
#include "common.h"

class Board;
typedef const Board Board_I;

// board configuration, not situation
// a board not processed by a transformation is called a raw board (raw_board), make this clear in function arguments!
// TODO: implement operator << for move
// TODO: use a better comparation rule so that rotations of a board are considered equal, otherwise unequal
class Board
{
public:
	Matrix<Who> m_data;

	// TODO: can these be non member ?
	static MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	static vector<Move> m_group; // a group of connected stones
	static Int m_qi; // qi of group

	Board() {}

	Board(Char_I Nx, Char_I Ny) : m_data(Nx, Ny)
	{
		m_data = Who::NONE; m_mark.resize(Nx, Ny); m_mark = 0;
	}

	inline Bool operator==(Board_I &rhs) const
	{
		return m_data == rhs.m_data;
	}

	inline void disp() const; // display board on screen

	inline Int check(Char_I x, Char_I y, Who_I s) const;

	inline Int place(Char_I x, Char_I y, Who_I s); // place stone

	inline void connect_init() const; // use before "connect()"

									  // get stones connected to a given stone (fill "m_group") and calculate qi
	inline void connect(Char_I x, Char_I y, Who_I who = Who::DEFAULT) const;

	// remove group if it is dead after placing (x,y)
	inline void remove_group();

	inline Int calc_territory2(Who_I who) const; // calculate black score (multiplied by 2)

	inline Bool is_eye(Char_I x, Char_I y, Who_I who) const;

	// check filling of an eye that's not in danger
	inline Bool is_dumb_eye_filling(Char_I x, Char_I y, Who_I who) const;

	// filling big eye surrounded by connected stones
	inline Bool is_dumb_2eye_filling(Char_I x, Char_I y, Who_I who) const;

	~Board() {}
};

inline void Board::disp() const
{
	Char i, x, y, Nx = board_Nx(), Ny = board_Ny();
	cout << "     ";
	for (i = 0; i < board_Nx(); ++i) cout << Int(i) << "   "; cout << "\n";
	cout << "    ";
	for (x = 0; x < Nx; ++x) cout << "----"; cout << "\n";
	for (y = 0; y < Ny; ++y) {
		cout << " " << Int(y) << " |";
		for (x = 0; x < Nx; ++x) {
			if (m_data(x, y) == Who::NONE)
				cout << "   |";
			else if (m_data(x, y) == Who::BLACK)
				cout << " @ |";
			else if (m_data(x, y) == Who::WHITE)
				cout << " O |";
			else
				error("Board::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "    ";
		for (i = 1; i <= Nx; ++i) cout << "----"; cout << "\n";
	}
}

// decide the best transformations (rotation and stone color flipping) to make a configuration "largest"
// NONE:0 < WHITE:1 < BLACK:2
// return true: if needs to flip
// return false: if no need to flip
// output rot: times the board needs to rotate counter-clockwise
Bool calc_rot_flip_board(Int_O &rotation, Board_I &board)
{
	Char x, y;
	Int i, Nx = board_Nx(), Ny = board_Ny(), N = Nx * Ny, best;
	vector<Int> val, rot;
	vector<Bool> flip;

	// square board (4 rotations)
	if (Nx == Ny) {
		flip.resize(8); rot.resize(8); val.resize(8);
	}
	else {
		flip.resize(4); rot.resize(4); val.resize(4);
	}

	// check board coordinates in row-major order
	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			if (Nx == Ny) {
				flip[0] = false; rot[0] = 0;
				flip[1] = false; rot[1] = 1;
				flip[2] = false; rot[2] = 2;
				flip[3] = false; rot[3] = 3;
				flip[4] = true;  rot[4] = 0;
				flip[5] = true;  rot[5] = 1;
				flip[6] = true;  rot[6] = 2;
				flip[7] = true;  rot[7] = 3;
			}
			else {
				flip[0] = false; rot[0] = 0;
				flip[1] = false; rot[1] = 2;
				flip[2] = true;  rot[2] = 0;
				flip[3] = true;  rot[3] = 2;
			}

			// evaluate (x, y) position for all transformations left
			best = 0;
			for (i = 0; i < val.size(); ++i) {
				val[i] = who2int(transform1(x, y, rot[i], flip[i], board));
				if (best < val[i])
					best = val[i];
			}

			// only keep the best transformations
			for (i = 0; i < val.size(); ++i) {
				if (val[i] < best) {
					flip.erase(flip.begin() + i);
					rot.erase(rot.begin() + i);
					val.erase(val.begin() + i);
				}
			}

			// only one transformations left
			if (val.size() == 1) {
				rotation = rot[0];
				return flip[0];
			}
		}
	}

	// multiple rot/flip combinations, return the first
	rotation = rot[0];
	return flip[0];
}

// get a stone after the board has been rotated and fliped
Who transform1(Char_I x, Char_I y, Int_I rot, Bool_I flip, Board_I &board)
{
	Char x1, y1;
	Who stone;
	if (rot == 0) {
		x1 = x; y1 = y;
	}
	else if (rot == 1) {
		x1 = -y; y1 = x;
	}
	else if (rot == 2) {
		x1 = -x; y1 = -y;
	}
	else if (rot == 3) {
		x1 = y; y1 = -x;
	}
	else
		error("illegal rotation!");

	stone = board.m_data(x1, y1);
	if (flip) {
		if (stone == Who::WHITE)
			return Who::BLACK;
		if (stone == Who::BLACK)
			return Who::WHITE;
	}
}

// compare two boards
// return 1: board > board_raw
// return 0: board == board_raw
// return -1: board < board_raw
// transformation will be performed on "board_raw", but not "board"
Int operator-(Board_I &board, Board_I &board_raw)
{
	// rotation and inversion should make the board compare as big as possible
	Char x, y;
	Int rotation, Nx = board_Nx(), Ny = board_Ny(), N = Nx * Ny;
	Int val, val_raw;
	Bool flip = calc_rot_flip_board(rotation, board_raw);

	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			val = who2int(board.m_data(x, y));
			val_raw = who2int(transform1(x, y, rotation, flip, board_raw));
			if (val == val_raw) continue;
			if (val > val_raw) return 1;
			return -1;
		}
	}
	return false;
}

// check if a placing is legal, or how many stones will be dead
// same check already exists for place()
// Ko is not considered!
// if legal, return the number of stones that can be removed ( >= 0)
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Board::check(Char_I x, Char_I y, Who_I who) const
{
#ifdef _CHECK_BOUND_
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Tree::place(x,y): out of bound!");
#endif
	Int Nx = board_Nx(), Ny = board_Ny(), Nremove = 0;
	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return -1;

	// search opponent's dead stones
	// only necessary if placed next to opposite stone
	connect_init();
	if (x > 0 && m_data(x - 1, y) == next(who)) {
		connect(x - 1, y);
		if (m_qi == 1)
			Nremove += m_group.size();
	}

	if (y > 0 && m_data(x, y - 1) == next(who) && !m_mark(x, y - 1)) {
		connect_init();
		connect(x, y - 1);
		if (m_qi == 1)
			Nremove += m_group.size();
	}

	if (x < Nx - 1 && m_data(x + 1, y) == next(who) && !m_mark(x + 1, y)) {
		connect_init();
		connect(x + 1, y);
		if (m_qi == 1)
			Nremove += m_group.size();
	}

	if (y < Ny - 1 && m_data(x, y + 1) == next(who) && !m_mark(x, y + 1)) {
		connect_init();
		connect(x, y + 1);
		if (m_qi == 1)
			Nremove += m_group.size();
	}

	if (Nremove > 0)
		return Nremove;

	// check qi assuming stone is placed
	connect_init();
	connect(x, y, who);
	if (m_qi == 0) return -2;

	return 0;
}

// return 0 if legal, and update board
// Ko is not considered! 
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Board::place(Char_I x, Char_I y, Who_I who)
{
#ifdef _CHECK_BOUND_
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Tree::place(x,y): out of bound!");
#endif
	Int Nx = board_Nx(), Ny = board_Ny();
	Bool removed = false;
	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return -1;

	// place stone
	m_data(x, y) = who;

	// remove opponent's dead stones
	// only necessary if placed next to opposite stone
	connect_init();
	if (x > 0 && m_data(x - 1, y) == next(who)) {
		connect(x - 1, y);
		if (m_qi == 0) {
			remove_group(); removed = true;
		}
	}

	if (y > 0 && m_data(x, y - 1) == next(who) && !m_mark(x, y - 1)) {
		connect_init();
		connect(x, y - 1);
		if (m_qi == 0) {
			remove_group(); removed = true;
		}
	}

	if (x < Nx - 1 && m_data(x + 1, y) == next(who) && !m_mark(x + 1, y)) {
		connect_init();
		connect(x + 1, y);
		if (m_qi == 0) {
			remove_group(); removed = true;
		}
	}

	if (y < Ny - 1 && m_data(x, y + 1) == next(who) && !m_mark(x, y + 1)) {
		connect_init();
		connect(x, y + 1);
		if (m_qi == 0) {
			remove_group(); removed = true;
		}
	}

	if (removed) return 0;

	// check qi of placed stone
	connect_init();
	connect(x, y);
	if (m_qi == 0) return -2;

	return 0;
}

inline void Board::connect_init() const
{
	m_mark = 0;
	m_group.resize(0);
	m_qi = 0;
}

// get a group of stones with the same color that are connected to stone (x,y)
// if (x, y) does not have a stone yet, specifying "who" can assume a stone is placed
inline void Board::connect(Char_I x, Char_I y, Who_I who /*optional*/) const
{
	Char Nx = board_Nx(), Ny = board_Ny();
	Who s, s0;
	if (m_data(x, y) == Who::NONE) {
		if (who == Who::DEFAULT)
			error("must specify who argument!");
		else
			s0 = who;
	}
	else
		s0 = m_data(x, y);
	m_group.push_back(Move(x, y));
	m_mark(x, y) = 1;

	if (x > 0) {
		s = m_data(x - 1, y);
		Char & mark = m_mark(x - 1, y);
		if (s == s0 && !mark)
			connect(x - 1, y);
		else if (s == Who::NONE && !mark) {
			++m_qi; mark = 1;
		}
	}

	if (x < Nx - 1) {
		s = m_data(x + 1, y);
		Char & mark = m_mark(x + 1, y);
		if (s == s0 && !mark)
			connect(x + 1, y);
		else if (s == Who::NONE && !mark) {
			++m_qi; mark = 1;
		}
	}

	if (y > 0) {
		s = m_data(x, y - 1);
		Char & mark = m_mark(x, y - 1);
		if (s == s0 && !mark)
			connect(x, y - 1);
		else if (s == Who::NONE && !mark) {
			++m_qi; mark = 1;
		}
	}

	if (y < Ny - 1) {
		s = m_data(x, y + 1);
		Char & mark = m_mark(x, y + 1);
		if (s == s0 && !mark)
			connect(x, y + 1);
		else if (s == Who::NONE && !mark) {
			++m_qi; mark = 1;
		}
	}
}

inline void Board::remove_group()
{
	Int i;
	for (i = 0; i < m_group.size(); ++i)
		m_data(m_group[i].x(), m_group[i].y()) = Who::NONE;
}

// this will be accurate when no legal move exists
// X territory = # X stone on board + # single qi's surrounded by X + (other qi's not surrounded by Y)/2
inline Int Board::calc_territory2(Who_I who) const
{
	Char x, y, Nx = board_Nx(), Ny = board_Ny();
	Int black = 0, qi = 0, common_qi = 0;
	Who s;
	for (x = 0; x < Nx; ++x)
		for (y = 0; y < Ny; ++y) {
			s = m_data(x, y);
			if (s == Who::NONE) {
				// qi not surrounded by black
				if ((x > 0 && m_data(x - 1, y) != who)
					|| (x < Nx - 1 && m_data(x + 1, y) != who)
					|| (y > 0 && m_data(x, y - 1) != who)
					|| (y < Ny - 1 && m_data(x, y + 1) != who)) {
					// qi not surrounded by white
					if ((x > 0 && m_data(x - 1, y) != next(who))
						|| (x < Nx - 1 && m_data(x + 1, y) != next(who))
						|| (y > 0 && m_data(x, y - 1) != next(who))
						|| (y < Ny - 1 && m_data(x, y + 1) != next(who)))
						++common_qi;
				}
				else // qi surrounded by black
					++qi;
			}
			else if (s == who)
				++black;
			else if (s != next(who))
				error("Board::result(): illegal stone!");
		}
	return 2 * (black + qi) + common_qi;
}

inline Bool Board::is_eye(Char_I x, Char_I y, Who_I who) const
{
	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return false;

	// check if not surrounded by the same color or board
	if (x > 0 && m_data(x - 1, y) != who ||
		x < board_Nx() - 1 && m_data(x + 1, y) != who ||
		y > 0 && m_data(x, y - 1) != who ||
		y < board_Ny() - 1 && m_data(x, y + 1) != who)
		return false;

	return true;
}

inline Bool Board::is_dumb_eye_filling(Char_I x, Char_I y, Who_I who) const
{
	// not my eye
	if (!is_eye(x, y, who)) return false;

	Int Nx = board_Nx(), Ny = board_Ny();

	// check qi of sourrounding 4 stones
	// if only 1 qi, it's not dumb

	connect_init();
	if (x > 0) {
		connect(x - 1, y);
		if (m_qi == 1) return false;
	}

	if (y > 0 && !m_mark(x, y - 1)) {
		connect_init();
		connect(x, y - 1);
		if (m_qi == 1) return false;
	}

	if (x < Nx - 1 && !m_mark(x + 1, y)) {
		connect_init();
		connect(x + 1, y);
		if (m_qi == 1) return false;
	}

	if (y < Ny - 1 && !m_mark(x, y + 1)) {
		connect_init();
		connect(x, y + 1);
		if (m_qi == 1) return false;
	}

	// it is dumb...
	return true;
}

inline Bool Board::is_dumb_2eye_filling(Char_I x, Char_I y, Who_I who) const
{
	Int Nx = board_Nx(), Ny = board_Ny();
	Int black;
	Char x1, y1, x_qi, y_qi;
	Bool qi = false, connected = false;

	// check qi of sourrounding 3 stones and one qi

	// check left
	x1 = x - 1; y1 = y;
	if (x > 0) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!m_mark(x1, y1)) return false; // black not connected
			}
			else {
				connect_init();
				connect(x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (qi) return false; // more than one qi
			qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check right
	x1 = x + 1; y1 = y;
	if (x < Nx - 1) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!m_mark(x1, y1)) return false; // black not connected
			}
			else {
				connect_init();
				connect(x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (qi) return false; // more than one qi
			qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check up
	x1 = x; y1 = y - 1;
	if (y > 0) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!m_mark(x1, y1)) return false; // black not connected
			}
			else {
				connect_init();
				connect(x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (qi) return false; // more than one qi
			qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check down
	x1 = x; y1 = y + 1;
	if (y < Ny - 1) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!m_mark(x1, y1)) return false; // black not connected
			}
			else {
				connect_init();
				connect(x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (qi) return false; // more than one qi
			qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check the other qi

	// left
	x1 = x_qi - 1; y1 = y_qi;
	if (x_qi > 0 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && m_mark(x1, y1)))
			return false;
	}

	// right
	x1 = x_qi + 1; y1 = y_qi;
	if (x_qi < Nx - 1 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && m_mark(x1, y1)))
			return false;
	}

	// up
	x1 = x_qi; y1 = y_qi - 1;
	if (y_qi > 0 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && m_mark(x1, y1)))
			return false;
	}

	// down
	x1 = x_qi; y1 = y_qi + 1;
	if (y_qi < Ny - 1 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && m_mark(x1, y1)))
			return false;
	}

	// ok, it is dumb...
	return true;
}
