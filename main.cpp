#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#ifndef NDEBUG
#define _RAND_SEED_ 0
#endif

#include "SLISC/slisc.h"
#include "SLISC/random.h"
#include "SLISC/search.h"
#include "SLISC/time.h"

using namespace slisc;
using std::vector; using std::string;
using std::ofstream; using std::cout;
using std::cin; using std::endl;

enum class Who : Char { NONE, BLACK, WHITE, DEFAULT, DRAW }; // TODO: make this a char type
typedef const Who Who_I;

enum class Act : Char { PLACE, PASS, EDIT, INIT }; // TODO: make this a char type
typedef const Act Act_I;

enum class Sol : Char { BAD, FAIR, GOOD, UNKNOWN, KO_ONLY };
typedef const Sol Sol_I;


// inverse color
inline Who next(Who_I s)
{
	if (s == Who::BLACK) return Who::WHITE;
	if (s == Who::WHITE) return Who::BLACK;
	if (s == Who::NONE) return Who::BLACK; // empty board
	error("opposite(Who_I): unknown error!");
}

// board size Nx, may only set once
inline Char board_Nx(Char_I Nx = -1)
{
	static Char Nx0 = -1;
	if (Nx0 < 0) {
		if (Nx > 0)
			Nx0 = Nx;
		else
			error("board_Nx(): Nx0 not uninitialized, or non-positive input!");
	}
	return Nx0;
}

// board size Ny, may only set once
inline Char board_Ny(Char_I Ny = -1)
{
	static Char Ny0 = -1;
	if (Ny0 < 0) {
		if (Ny > 0)
			Ny0 = Ny;
		else
			error("board_Ny(): size must be positive!");
	}
	return Ny0;
}

// board size Ny, may only set once
// should only be integer or half integers
inline Int komi2(Int_I k2 = -1132019)
{
	static Doub k20 = -1132019;
	if (k2 != -1132019) {
		if (abs(k2) > 2 * board_Nx() * board_Ny())
			error("komi too large or too small! One side always win!");
		k20 = k2; // set komi
	}
	else if (k20 == -1132019)
		error("komi(): komi unset!");
	return k20;
}

// complementary territory (opponent's territory)
inline Int comp_territory2(Int_I territory2)
{
	return 2 * board_Nx()*board_Ny() - territory2;
}

// complementary score (opponent's territory)
inline Int comp_score2(Int_I score2)
{
	return comp_territory2(score2);
}

// complementary solution (opponent's solution)
inline Sol comp_sol(Sol_I sol)
{
	if (sol == Sol::GOOD)
		return Sol::BAD;
	if (sol == Sol::BAD)
		return Sol::GOOD;
	// KO_ONLY or UNKNOWN
	return sol;
}

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
	inline Move(Char_I x, Char_I y);
	inline Move(Act_I act);

	// properties
	Bool isplace() const { return m_x >= 0; }
	Bool ispass() const { return m_x == -1; }
	Bool isedit() const { return m_x == -2; }
	Bool isinit() const { return m_x == -3; }
	Act type() // 0: isplace(), 1: ispass(), 2: isedit(), 3: isinit()
	{ return isplace()? Act::PLACE: ispass()? Act::PASS: isedit()? Act::EDIT: Act::INIT; }
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
	if ( m_x < 0 ) error("Move::x(): not a coord!");
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

// a node in the game tree
// a fork index (forkInd) is an index for m_last or m_next
// Move is the move that caused the current board
class Node: public Move
{
private:
	Who m_who; // who's turn is this?
	vector<Long> m_last; // -1: first node
	vector<Long> m_next; // -1: end node
	Long m_poolInd; // pool index, board stored in Pool::m_board[m_pool_ind]
public:
	// solution related
	Sol m_sol; // Sol::GOOD/Sol::BAD/Sol::FAIR
	Int m_score2; // an over-estimation of final score (two gods playing), -1 means unclear
	Sol m_best_child_sol; // solution of the best child that are already solved
	Int m_best_child_sco2; // score of the best child that are already solved
	Bool m_all_child_exist; // all legal children exist in the tree

	Node(): m_sol(Sol::UNKNOWN), m_best_child_sol(Sol::BAD), m_best_child_sco2(-1), m_all_child_exist(false), m_score2(-1) {}
	
	// properties
	using Move::isplace;
	using Move::ispass;
	using Move::isedit;
	using Move::isinit;
	using Move::type;
	using Move::x;
	using Move::y;
	const Move & move() const { return *this; }
	Who who() const { return m_who; }
	Long poolInd() const { return m_poolInd; }
	Int nlast() const { return m_last.size(); }
	Long last(Int_I ind) const { return m_last[ind]; }
	Int nnext() const { return m_next.size(); }
	inline Long next(Int_I ind) const; // tree index for a child
	inline Bool isend() const; // is this a bottom node (end of game)?
	inline Bool is_next_ko(Int_I ind) const; // is this a ko link?

	// set
	void init(Long_I poolInd) { Move::init(); m_last.resize(0); m_who = Who::NONE; m_poolInd = poolInd; }
	void push_last(Long_I treeInd) { m_last.push_back(treeInd); }
	void push_next(Long_I treeInd) { m_next.push_back(treeInd); }

	void place(Char_I x, Char_I y, Who_I who, Long_I treeInd, Long_I poolInd) // already has bound checking
	{ Move::place(x, y); m_who = who; push_last(treeInd); m_poolInd = poolInd; }

	void pass(Who_I who, Long_I treeInd, Long_I poolInd)
	{ Move::pass(); push_last(treeInd); m_poolInd = poolInd; m_who = who; }

	~Node() {}
};

// ind = -1 : last element, ind = -2 : second last element, etc.
Long Node::next(Int_I ind) const
{
	Long treeInd;
	if (ind < 0)
		treeInd = m_next[nnext() + ind];
	else
		treeInd = m_next[ind];

	if (treeInd > 0)
		return treeInd;
	else if (treeInd < -1)
		return -treeInd - 1;
	else
		error("unkown!");
}

inline Bool Node::isend() const
{
	if (m_next.size() == 1 & m_next[0] == -1)
		return true;
	return false;
}

// ind = -1 : last element, ind = -2 : second last element, etc.
inline Bool Node::is_next_ko(Int_I ind) const
{
	Long treeInd;
	if (ind < 0)
		treeInd = m_next[nnext() + ind];
	else
		treeInd = m_next[ind];

	if (treeInd < -1)
		return true;
	else if (treeInd > 0)
		return false;
	else
		error("unknown!");
}

class Board;
typedef const Board Board_I;

// status of the board
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

	Board(Char_I Nx, Char_I Ny): m_data(Nx, Ny)
	{ m_data = Who::NONE; m_mark.resize(Nx, Ny); m_mark = 0; }

	inline Bool operator==(Board_I &rhs) const
	{ return m_data == rhs.m_data; }

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
			if (m_data(x,y) == Who::NONE)
				cout << "   |";
			else if (m_data(x,y) == Who::BLACK)
				cout << " @ |";
			else if (m_data(x,y) == Who::WHITE)
				cout << " O |";
			else
				error("Board::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "    ";
		for (i = 1; i <= Nx; ++i) cout << "----"; cout << "\n";
	}
}

// imagine Board::m_data as a long integer, try to compare two integers
inline Bool operator<(Board_I &board1, Board_I &board2)
{
	Int i, N = board_Nx() * board_Ny();
	for (i = 0; i < N; ++i) {
		if (board1.m_data(i) == board2.m_data(i)) continue;
		if (board1.m_data(i) < board2.m_data(i)) return true;
		return false;
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
	if (m_data(x,y) != Who::NONE)
		return -1;

	// place stone
	m_data(x,y) = who;

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

// all situations in the tree, sorted for quick search
// sorting: each board is a radix 3 number, sort these numbers with ascending order
// a pool index (poolInd) is an index for m_boards, this index should never change for the same board
// m_treeInd will link to a tree node that is not a Act::PASS
// index to m_order is called (pool) order index (orderInd), this will change frequently for the same board!
class Pool
{
public:
	vector<Board> m_boards; // store all boards in the Pool
	vector<Long> m_treeInd;  // m_treeInd[i] is the index of m_boards[i] in the game tree
	vector<Long> m_order; // m_board[m_order[i]] is in sorted order

	Pool() {}

	Long size() const { return m_boards.size(); }

	// get a board by index
	const Board & operator[](Long_I ind) const { return m_boards[m_order[ind]]; }

	// search Pool: find ind so that *this[ind] == board
	// TODO: improve implementation (stone by stone algorithm)
	Int search(Long_O &treeInd, Long_O &orderInd, Board_I &board);

	// add a new board to the Pool
	inline void push(Board_I &board, Int_I search_ret, Long_I orderInd, Long_I treeInd);
};

Int Pool::search(Long_O &treeInd_same, Long_O &orderInd, Board_I &board)
{
	Int ret = lookupInt(orderInd, *this, board);
	if (ret == 0)
		treeInd_same = m_treeInd[m_order[orderInd]];
	return ret;
}

// orderInd is output by search() and search_ret is returned by search()
// orderInd should be -3, -2, -1 or 1
inline void Pool::push(Board_I &board, Int_I search_ret, Long_I orderInd, Long_I treeInd)
{
	m_boards.push_back(board);
	m_treeInd.push_back(treeInd);
	Long poolInd = m_boards.size()-1;
	if (search_ret == -2)
		m_order.insert(m_order.begin() + orderInd + 1, poolInd);
	else if (search_ret == -1)
		m_order.insert(m_order.begin(), poolInd);
	else if (search_ret == 1 || search_ret == -3)
		m_order.push_back(poolInd);
	else
		error("Pool::push(): unknown search_ret!");
}

// game tree
// a tree index is an index for m_nodes (treeInd), this index should never change for the same node
// if any node has nlast() > 1, it creates an "upward fork"
// TODO: make sure no two nodes have the same situation
// TODO: treeInd = 0 should be the empty board!
class Tree
{
private:
	Long m_treeInd; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_nodes;
	Pool m_pool;

public:
	Tree();

	// global work space, should not be a member
	vector<Long> m_ko_treeInds; // keep track of all the destinations of Ko links
	Sol m_ko_best; // best descendent solution except ko

	// peoperties
	const Board & get_board(Long_I treeInd = -1) const // reference to the Board obj of a node
	{
		Long treeInd1 = def(treeInd);
		Long poolInd = m_nodes[treeInd1].poolInd();
		return m_pool.m_boards[poolInd];
	}

	Long index() const { return m_treeInd; } // return current node index

	Long nnode() const { return m_nodes.size(); } // return number of nodes in the tree

	inline Bool isend(Long_I treeInd = -1) const; // if a node is an end node

	inline Bool is_next_ko(Long_I treeInd = -1, Int_I forkInd = 0) const;

	inline Node & node(Long_I treeInd = -1) // return a reference of a node
	{ return m_nodes[def(treeInd)]; }

	inline const Node & node(Long_I treeInd = -1) const // return a reference of a node
	{ return m_nodes[def(treeInd)]; }

	inline Long last(Long_I treeInd = -1, Int_I forkInd = 0) const; // return an index of the last node of a node

	const Node & lastNode(Long_I treeInd = -1, Int_I forkInd = 0) // return a reference of the last node of a node
	{ return m_nodes[last(treeInd, forkInd)]; }

	// return an index of the next node of a node, fork can be negative
	inline Long next(Long_I treeInd = -1, Int_I forkInd = 0) const;

	const Node & nextNode(Long_I treeInd = -1, Int_I forkInd = 0) // return a reference of the next node of a node
	{ return m_nodes[next(treeInd, forkInd)]; } // forkInd can be negative

	Who who(Long_I treeInd = -1) const // who played the node
	{ return m_nodes[def(treeInd)].who(); }

	inline void disp_board(Long_I treeInd = -1) const; // display board

	inline Int pass(Long_I treeInd = -1);

	inline Int check(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int place(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int islinked(Long_I treeInd1, Long_I treeInd2); // check if node treeInd1 can lead to node treeInd2

	// check if same board already exists in the Pool and decide if it is a Ko
	inline Int check_ko(Int_O &search_ret, Long_O &treeInd_same, Long_O &orderInd, Long_I treeInd, Board_I &board);
	
	inline void branch(vector<Long> &br, Long_I treeInd = -1); // get an index vector of the a branch ending at a node

	inline void writeSGF_old(const string &name); // write a branch to file

	inline void writeSGF(const string &name); // write the whole tree to file

	inline Long writeSGF0(ofstream &fout, Long_I treeInd = -1); // internal function called by writeSGF();

	inline void writeSGF01(ofstream &fout, Long_I treeInd, const string &prefix = ""); // internal function called by writeSGF0();

	inline Bool next_exist(Move mov, Long_I treeInd = -1); // does next node have this mov already?

	// make a random move for a node
	// only pass if all other moves are onsidered
	inline Int rand_move(Long_I treeInd = -1);

	// smarter random move for a node
	// will not fill single eye unless it could be destroyed immediately, or if all other moves already exists
	inline Int rand_smart_move(Long_I treeInd = -1);

	// randomly play to the end of the game from a node (default: from top node)
	inline Int rand_game(Long_I treeInd = 0, Bool_I out = false);

	inline void solve_end(Long_I treeInd = -1); // solve a bottom node

	inline Int solve(Long_I treeInd = -1); // analyse who has winning strategy for a node

	Who winner(Long treeInd = -1);

	void calc_sol(Long_I treeInd = -1); // update m_sol based on m_territory2 and komi

	void calc_score(Long_I treeInd = -1); // calculate territory and save to m_territory2

	Int score2(Long_I treeInd = -1) const
	{
		Int sco2 = node(def(treeInd)).m_score2;
		if (sco2 < 0 || sco2 > 2 * board_Nx() * board_Ny())
			error("illegal score");
		return sco2;
	}

	void set_score2(Int score2, Long_I treeInd = -1)
	{
		if (score2 < 0 || score2 > 2 * board_Nx() * board_Ny())
			error("illegal score");
		node(def(treeInd)).m_score2 = score2;
	}

	Sol solution(Long_I treeInd = -1) const
	{ return node(def(treeInd)).m_sol; }

	void set_solution(Sol_I sol, Long_I treeInd = -1)
	{ node(def(treeInd)).m_sol = sol; }

	Bool solved(Long_I treeInd = -1) const
	{
		Sol sol = solution(def(treeInd));
		if (sol == Sol::GOOD || sol == Sol::BAD || sol == Sol::FAIR)
			return true;
		return false;
	}

	Sol & best_child_sol(Long_I treeInd = -1)
	{ return node(def(treeInd)).m_best_child_sol; }

	Int & best_child_sco2(Long_I treeInd = -1)
	{ return node(def(treeInd)).m_best_child_sco2; }

	Bool & all_child_exist(Long_I treeInd = -1)
	{ return m_nodes[def(treeInd)].m_all_child_exist; }

	// set default argument treeInd
	Long def(Long_I treeInd) const
	{ return treeInd < 0 ? m_treeInd : treeInd; }

	~Tree() {}
};

// create 0-th node: empty board
Tree::Tree() : m_treeInd(0)
{
	Board board(board_Nx(), board_Ny());
	m_pool.push(board, -3, 0, 0);
	Node node; node.init(m_pool.size()-1);
	m_nodes.push_back(node);
}

inline Bool Tree::isend(Long_I treeInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].isend())
			return true;
	return false;
}

inline Bool Tree::is_next_ko(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].is_next_ko(forkInd))
		return true;
	return false;
}

inline Long Tree::last(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	const Node &node = m_nodes[def(treeInd)];
	return node.last(forkInd);
}

inline Long Tree::next(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	return  m_nodes[def(treeInd)].next(forkInd);
}

inline void Tree::disp_board(Long_I treeInd /*optional*/) const
{
	Char x, y;
	Long treeInd1 = def(treeInd);
	const Node & node = m_nodes[treeInd1];
	if (node.isinit())
		cout << "(empty)";
	else {
		if (who(treeInd1) == Who::BLACK)
			cout << "(black ";
		else if (who(treeInd1) == Who::WHITE)
			cout << "(white ";
		else
			error("Tree:disp(): illegal side name!");

		if (node.ispass())
			cout << "pass)";
		else if (node.isedit())
			cout << "edit board)";
		else
			cout << "[" << Int(node.x()) << "," << Int(node.y()) << "] )";
	}
	
	cout << "\n";
	get_board(treeInd1).disp();
	cout << '\n' << endl;
};

// return -1 if game ends, 0 if not
inline Int Tree::pass(Long_I treeInd /*optional*/)
{
	Node node;
	const Long treeInd1 = def(treeInd);
	node.pass(::next(who(treeInd1)), treeInd1, m_nodes[treeInd1].poolInd());
	m_nodes.push_back(node);
	m_nodes[treeInd1].push_next(nnode()-1);
	// check double pass (game ends)
	if (m_nodes[treeInd1].ispass()) {
		m_nodes[nnode() - 1].push_next(-1);
		// check result
		solve_end(nnode() - 1);
		return -1;
	}
	if(treeInd < 0) ++m_treeInd;
	return 0;
}

// check if a placing is legal, or how many stones will be dead
// same check already exists for place()
// Ko is not considered!
// if legal, return the number of stones that can be removed ( >= 0)
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Tree::check(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = def(treeInd);
	Node node;
	Board board;
	board = get_board(treeInd1);
	// first move
	if (treeInd1 == 0)
		return 0;

	// update board and check illegal move (Ko no checked!)
	return board.check(x, y, ::next(who(treeInd1)));
}

// return 0 if new node created (m_treeInd++)
// return 1 if linked to an existing node (m_treeInd jumped)
// return -1 if illegal (nothing changes)
// return -2 if a ko is found and linked (m_treeInd unchanged)
// already has bound checking
inline Int Tree::place(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = def(treeInd);
	Node node;
	Board board;
	board = get_board(treeInd1);
	// first move
	if (treeInd1 == 0) {
		board.place(x, y, Who::BLACK);
		m_pool.push(board, -3, 0, treeInd1);
		node.place(x, y, Who::BLACK, treeInd1, m_pool.size()-1);
		m_nodes.push_back(node);
		m_nodes[treeInd1].push_next(nnode() - 1);
		if (treeInd < 0) m_treeInd = 1;
		return 0;
	}

	// update board and check illegal move (Ko no checked!)
	if (board.place(x, y, ::next(who(treeInd1))))
		return -1;

	// check Ko
	Int search_ret;
	Long orderInd;
	Long treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd1, board);
	if (ret < 0) { // board already exists
		if (ret == -1) { // not a ko
			m_nodes[treeInd1].push_next(treeInd_found);
			if (treeInd < 0) m_treeInd = treeInd_found;
			return 1;
		}
		else if (ret == -2) { // is a ko
			m_nodes[treeInd1].push_next(-treeInd_found - 1);
			return -2;
		}
		// ret == -3: not the same player, continue
	}
	
	// board is new
	if (ret == -3) { // no need to push
		node.place(x, y, ::next(who(treeInd1)), treeInd1, m_nodes[treeInd_found].poolInd());
	}
	else { // push board to pool
		m_pool.push(board, search_ret, orderInd, treeInd1 + 1);
		node.place(x, y, ::next(who(treeInd1)), treeInd1, m_pool.size() - 1);
	}

	// add Node to tree
	m_nodes.push_back(node);
	m_nodes[treeInd1].push_next(nnode() - 1);
	if (treeInd<0) ++m_treeInd;
	return 0;
}

// return 0 if not linked
// return 1 if node treeInd1 can lead to node treeInd2
// upward search from node treeInd2 is most efficient
// this is a recursive function to deal with upward fork
inline Int Tree::islinked(Long_I treeInd1, Long_I treeInd2)
{
	Long i, treeInd = treeInd2;
	for (i = 0; i < 10000; ++i) {
		if (m_nodes[treeInd].nlast() > 1) break; // multiple upward fork
		if (treeInd == treeInd1) return 1; // found treeInd2
		if (treeInd == 0) return 0; // reached top of tree
		treeInd = m_nodes[treeInd].last(0); // single line, go up
	}
	for (i = 0; i < m_nodes[treeInd].nlast(); ++i) {
		if (islinked(treeInd1, last(treeInd, i)) == 1)
			return 1;
	}
	error("should not reach here!"); return 0;
}

// 'nodes[treeInd]' will produce 'board' in the next move
// if board already exists, output tree index of the same board
// return 0 if board is new
// return -1 if it is not a ko
// return -2 if a ko is found
// return -3 if board exist but player is different (solution/score not the same!)
// (in case of multiple upward fork, there might be no ko for some branches!)
inline Int Tree::check_ko(Int_O &search_ret, Long_O &treeInd_same, Long_O &orderInd, Long_I treeInd, Board_I &board)
{
	search_ret = m_pool.search(treeInd_same, orderInd, board);
	// board already exists
	if (search_ret == 0) {
		if (who(treeInd_same) == who(treeInd))
			return -3;
		if (islinked(treeInd_same, treeInd))
			return -2; // Ko!
		return -1; // no Ko
	}
	return 0;
}

// if multiple paths exists choose the first one for now
inline void Tree::branch(vector<Long> &br, Long_I treeInd /*optional*/)
{
	Int i, Nbr;
	Long treeInd1 = def(treeInd);
	br.resize(0);
	for (i = 0; i < 10000; ++i) {
		if (treeInd1 == 0) break;
		br.push_back(treeInd1);
		if (m_nodes[treeInd1].nlast() == 0) error("unknown error!");
		treeInd1 = m_nodes[treeInd1].last(0);
	}
	Nbr = br.size();
	for (i = Nbr / 2 - 1; i > -1; --i) {
		SWAP(br[Nbr - i - 1], br[i]);
	}
}

inline void Tree::writeSGF_old(const string &name)
{
	Char Nx = board_Nx(), Ny = board_Ny();
	ofstream fout(name);
	fout << "(\n";
	fout << ";GM[1]FF[4]CA[UTF-8]AP[]KM[0]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Int(Nx) << ":" << Int(Ny) << "]";
	fout << "DT[]\n";

	// write a branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Long> br;
	branch(br);
	for (i = 0; i < br.size(); ++i) {
		// black moves
		fout << "  ;" << BW;
		Node & node = m_nodes[br[i]];
		if (node.ispass()) // pass
			fout << "[]\n";
		else
			fout << '[' << char('a' + node.x()) << char('a' + node.y()) << "]\n";
		BW = BW == 'B' ? 'W' : 'B';
	}
	fout << ")\n";
	fout.close();
}

inline void Tree::writeSGF(const string &name)
{
	Char Nx = board_Nx(), Ny = board_Ny();
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[" << 0.5*komi2() << "]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Int(Nx) << ":" << Int(Ny) << "]";
	fout << "DT[]\n";

	Char BW; // letter B or letter W
	Int i; Char x, y;
	
	fout.flush(); // debug
	Long max_treeInd =  writeSGF0(fout, 1);
	if (max_treeInd != nnode() - 1)
		error("writeSGF() nodes number does not match!");
	
	fout << ")\n";
	fout.close();
}

// return max node index written
// nodes should be written in the order of treeIndex
// recursively write the tree
inline Long Tree::writeSGF0(ofstream &fout, Long_I treeInd)
{
	Int i, nnext;
	Long treeInd1 = def(treeInd), treeInd2;
	static Long max_treeInd = treeInd1 - 1;
	if (treeInd == 1) max_treeInd = 0;

	// write one node
	writeSGF01(fout, treeInd1);
	++max_treeInd;

	// go down the branch if no fork
	for (i = 0; i < 10000; ++i) {
		nnext = m_nodes[treeInd1].nnext();
		if (nnext == 1) {
			if (isend(treeInd1)) {
				return max_treeInd; // game ends
			}
			treeInd2 = next(treeInd1);
			if (is_next_ko(treeInd1)) {
				writeSGF01(fout, treeInd2, "ko>");
				return max_treeInd; // ko link
			}
			if (treeInd2 < treeInd1) { // reached a link to an existing node
				writeSGF01(fout, treeInd2, ">"); // write a node to represent a link
				return max_treeInd; 
			}
			if (treeInd2 != treeInd1 + 1)
				error("unknown!");
			treeInd1 = treeInd2;
			writeSGF01(fout, treeInd1);
			++max_treeInd;
		}
		else if (nnext == 0)
			return max_treeInd;
		else if (nnext > 1) {
			break; // reached downward fork
		}
	}

	// write downward branches
	for (i = 0; i < nnext; ++i) {
		fout << '(';
		if (isend(treeInd1))
			error("unkown error"); // game ends or ko link!
		treeInd2 = next(treeInd1, i);
		if (is_next_ko(treeInd1, i)) {
			writeSGF01(fout, treeInd2, "ko>"); // write a node to represent a link
			continue;
		}
		if (treeInd2 < max_treeInd) { // reached a link to an existing node
			writeSGF01(fout, treeInd2, ">"); // write a node to represent a link
			continue;
		}
		if (treeInd2 != max_treeInd + 1)
			error("unknown!");

		writeSGF0(fout, treeInd2);
		fout << ')';
	}
	return max_treeInd;
}

// write one node to SGF file
inline void Tree::writeSGF01(ofstream &fout, Long_I treeInd1, const string &prefix)
{
	Char BW; // letter B or letter W
	Node & node = m_nodes[treeInd1];
	Int i, nnext;
	if (who(treeInd1) == Who::BLACK)
		BW = 'B';
	else if (who(treeInd1) == Who::WHITE)
		BW = 'W';
	else
		error("illegal stone color");

	fout << ";" << BW;
	if (node.ispass()) // pass
		fout << "[]";
	else
		fout << '[' << char('a' + node.x()) << char('a' + node.y()) << "]";

	// add node number to title
	fout << "N[" << prefix << "[" << treeInd1 << "\\]";
	// add score
	if (solved(treeInd1))
		fout << 0.5*score2(treeInd1);
	// end title
	fout << "]";

	// add green (black wins) or blue (white wins) mark
	if (winner(treeInd1) == Who::BLACK)
		fout << "TE[1]"; // green
	else if (winner(treeInd1) == Who::WHITE)
		fout << "IT[]"; // blue
	else if (winner(treeInd1) == Who::DRAW)
		error("TODO...");
	else if (winner(treeInd1) == Who::NONE)
		;
	else
		error("???");

	fout << "\n";
}

inline Bool Tree::next_exist(Move mov, Long_I treeInd /*optional*/)
{
	Int i;
	Long treeInd1 = def(treeInd);
	Node &node = m_nodes[treeInd1];
	for (i = 0; i < node.nnext(); ++i) {
		if (nextNode(treeInd1, i).move() == mov)
			return true;
	}
	return false;
}

// return 0 if successful and new node created
// return 1 if linked to existing node and no Ko
// return 2 if linked to existing node and has Ko
// return -1 if there is all legal moves already exists
// return -2 if double pass caused end of game
Int Tree::rand_move(Long_I treeInd /*optional*/)
{
	error("update this function from rand_smart_move()");
}

// return 0 if successful and new node created
// return 1 if linked to existing node and no Ko
// return 2 if linked to existing node and has Ko
// return 3 if sucessful but is a dumb move
// return -1 if there is all legal moves already exists
// return -2 if passed and game ends
Int Tree::rand_smart_move(Long_I treeInd /*optional*/)
{
	Bool exist, exist_pass = false;
	Int i, j, ret, Nx = board_Nx(), Ny = board_Ny(), Nxy = Nx*Ny;
	Char x0, y0, x, y;
	VecInt xy;
	vector<Int> dumb_xy;
	Long_I treeInd1 = def(treeInd);
	Node & node = m_nodes[treeInd1];

	// random sequence of all grid points on board
	randPerm(xy, Nxy);

	// search xy for a new legal move (not in Node::m_next)
	for (i = 0; i < Nxy; ++i) {
		x = xy[i] % Nx; y = xy[i] / Nx;
		// check existence
		if (next_exist(Move(x, y), treeInd1))
			continue;
		// check legal and number of removal
		ret = check(x, y, treeInd1);
		if (ret < 0)
			continue;
		// check dumb eye filling
		if (get_board(treeInd1).is_dumb_eye_filling(x, y, ::next(who(treeInd1)))) {
			dumb_xy.push_back(i);
			continue;
		}
		if (get_board(treeInd1).is_dumb_2eye_filling(x, y, ::next(who(treeInd1)))) {
			dumb_xy.push_back(i);
			continue;
		}
		ret = place(x, y, treeInd1);
		if (ret == -1)
			continue; // illegal move, no change
		else if (ret == 0)
			return 0; // new node created
		else if (ret == 1)
			return 1;  // linked to an existing node, no Ko
		else if (ret == -2)
			return 2; // linked to existing node, has Ko
	}

	// no non-dumb placing left, consider passing
	if (!next_exist(Move(Act::PASS), treeInd1)) {
		if (pass(treeInd1)) return -2; // double pass
		else return 0; // first pass
	}

	// consider doing dumb move
	Nxy = dumb_xy.size();
	for (i = 0; i < Nxy; ++i) {
		x = xy[i] % Nx; y = xy[i] / Nx;
		if (place(x, y) < 0) continue;
		else return 3;
	}

	return -1; // all leagl moves already exist
}

// m_treeInd will be changed to the end node!
// the end node will have the correct Node::m_win
// using rand_move1() currently
// set "out = true" for output to terminal and test.sgf
inline Int Tree::rand_game(Long_I treeInd, Bool_I out)
{
	Int i, ret;

	m_treeInd = treeInd;
	
	// debug: edit board here
	/*tree.place(0, 0); tree.place(1, 2);
	tree.place(2, 2); tree.place(1, 1);
	tree.place(0, 2); tree.place(0, 1);
	tree.place(2, 1); tree.place(1, 0);
	tree.pass();*/
	// end edit board

	if (out) disp_board();

	for (i = 1; i < 10000; ++i) {
		ret = rand_smart_move();
		if (ret == -1) {
			if (out) cout << "all moves exists\n\n\n";
			break;
		}
		if (ret == -2) {
			if (out) cout << "double pass !\n\n\n";
			break;
		}
		if (out) cout << "step " << i << endl;
		if (out) disp_board();
	}
	
	if (out) cout << "game over!" << "\n\n";
	
	solve_end();
	
	if (winner() == Who::BLACK) {
		if (out) cout << "black wins!";
		if (out) cout << "  (score: " << 0.5*score2() << ")\n\n";
	}
	else if (winner() == Who::WHITE) {
		if (out) cout << "white wins!";
		if (out) cout << "  (score: " << 0.5*score2() << ")\n\n";
	}
	else { // draw
		if (out) cout << "draw!\n\n";
	}
	if (out) writeSGF("randdom_game.sgf");

	return 0;
}

inline void Tree::solve_end(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);

	// TODO check if this is an end node
	if (!isend(treeInd1))
		error("Tree::solve_end(): unkown error!");

	calc_score(treeInd1); calc_sol(treeInd1);
}

// might m_treeInd be changed? it shouldn't
// this is a recursive function
// using rand_smart_move() to generate moves until a better one is available
// assuming there is no branch after treeInd for now
// return 0 if successful
// return -1 if at least one downstream ko exists, and all other descendents have been solved
//           all downstream ko stored in Tree::m_ko_treeInds, best downstream solution stored in Tree::m_ko_best

Int Tree::solve(Long_I treeInd /*optional*/)
{
	Int i, ret, child_treeInd;
	Long treeInd1 = def(treeInd);
	Bool found_ko = false;
	Int child_sco2;
	Sol child_sol;

	// if already solved
	if (solution(treeInd1) != Sol::UNKNOWN) {
		if (solution(treeInd1) == Sol::KO_ONLY) {
			error("unhandled situation!");
		}
		return 0;
	}

	// enumerate children
	for (i = 0; i < 1000; ++i) {
		ret = rand_smart_move(treeInd1);
		// debug
		if (nnode() == 179) {
			writeSGF("test.sgf");
		}
		// end debug
		if (ret == 0 || ret == 1 || ret == 3) {
			// new node created (might be a first pass)
			if (ret == 0 || ret == 3) {
				child_treeInd = nnode() - 1;
				// debug, display board
				cout << "largest treeInd = " << nnode() - 1 << endl;
				disp_board(nnode() - 1); cout << "\n\n" << endl;
				cout << ""; // debug break point
				// end debug
			}
			// linked to existing node, no Ko
			else if (ret == 1) {
				child_treeInd = next(treeInd1, -1);
			}
			else
				error("unhandled case!");

			ret = solve(child_treeInd);
			if (ret == -1) {
				continue; // downstream ko exists
			}
			child_sco2 = score2(child_treeInd);
			child_sol = solution(child_treeInd);

			if (best_child_sco2(treeInd1) < child_sco2) {
				best_child_sco2(treeInd1) = child_sco2;
				best_child_sol(treeInd1) = child_sol;
				if (best_child_sol(treeInd1) == Sol::GOOD) {
					set_solution(Sol::BAD, treeInd1);
					set_score2(comp_score2(best_child_sco2(treeInd1)), treeInd1);
					return 0; // debug break point
				}
			}
		}
		// linked to existing node, has Ko
		else if (ret == 2) {
			Long child_treeInd = next(treeInd1, -1);
			m_ko_treeInds.push_back(child_treeInd);
			found_ko = true;
			continue;
		}
		// all children exist
		else if (ret == -1) {
			all_child_exist(treeInd1) = true;
			// all children solved
			if (!found_ko) {
				if (best_child_sol(treeInd1) == Sol::FAIR) {
					set_solution(Sol::FAIR, treeInd1);
					set_score2(comp_score2(best_child_sco2(treeInd1)), treeInd1);
				}
				else { // best == Sol::BAD
					set_solution(Sol::GOOD, treeInd1);
					set_score2(comp_score2(best_child_sco2(treeInd1)), treeInd1);
				}
				return 0; // debug break point
			}
			// not all children solved
			else {
				set_solution(Sol::KO_ONLY, treeInd1);
				set_score2(comp_score2(best_child_sco2(treeInd1)), treeInd1);
				return -1; // debug break point
			}
		}
		// new pass node created, double pass caused end of game
		else if (ret == -2) {
			// child is already solved by game result
			// debug, display board
			cout << "largest treeInd = " << nnode() - 1 << endl;
			disp_board(nnode() - 1); cout << "\n\n" << endl;
			// end debug
			child_treeInd = nnode() - 1; // debug break point
			if (best_child_sco2(treeInd1) < score2(child_treeInd)) {
				best_child_sco2(treeInd1) = score2(child_treeInd);
				best_child_sol(treeInd1) = solution(child_treeInd);
				if (best_child_sol(treeInd1) == Sol::GOOD) {
					set_solution(Sol::BAD, treeInd1);
					set_score2(comp_score2(best_child_sco2(treeInd1)), treeInd1);
					return 0; // debug break point
				}
			}
		}
		else
			error("unhandled return!");
	}
	error("unkown error!"); return -1;
}

Who Tree::winner(Long treeInd)
{
	Long treeInd1 = def(treeInd);
	Sol sol = solution(treeInd1);
	Who player = who(treeInd1);
	
	if (sol == Sol::UNKNOWN)
		return Who::NONE;
	if (player == Who::BLACK) {
		if (sol == Sol::GOOD)
			return Who::BLACK;
		if (sol == Sol::BAD)
			return Who::WHITE;
		if (sol == Sol::FAIR)
			return Who::DRAW;
	}
	else if (player == Who::WHITE) {
		if (sol == Sol::GOOD)
			return Who::WHITE;
		if (sol == Sol::BAD)
			return Who::BLACK;
		if (sol == Sol::FAIR)
			return Who::DRAW;
	}
	else
		error("illegal Who input");
}

void Tree::calc_sol(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);

	Int score4_draw = board_Nx()*board_Ny() * 2;
	if (who(treeInd1) == Who::BLACK)
		score4_draw += komi2();
	else if (who(treeInd1) == Who::WHITE)
		score4_draw -= komi2();
	else
		error("illegal player!");
	Int score4 = 2 * score2(treeInd1);
	if (score4 > score4_draw)
		set_solution(Sol::GOOD, treeInd1);
	else if (score4 < score4_draw)
		set_solution(Sol::BAD, treeInd1);
	else
		set_solution(Sol::FAIR, treeInd1);
}

void Tree::calc_score(Long_I treeInd) // calculate territory and save to m_territory2
{
	Long treeInd1 = def(treeInd);
	node(treeInd1).m_score2 = get_board(def(treeInd1)).calc_territory2(who(treeInd1));
}

// initialize class static members
MatChar Board::m_mark; // 0: unmarked, 1: marked, else: not used yet
vector<Move> Board::m_group; // a group of connected stones
Int Board::m_qi; // qi of group

// run this for stand along program
void computer_vs_computer_ui()
{
	Int Nx, Ny;
	Doub komi;
	cout << "random game..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;
	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	tree.rand_game(0, true);
	getchar();
	getchar();
}

// run this for stand alone human vs computer
void human_vs_computer_ui()
{
	Int Nx, Ny, ret, i = 0, bscore4, x, y;
	Doub komi;
	Char color;
	cout << "human vs computer..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;
	cout << "your color : "; cin >> color;

	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	i = 0;
	cout << "\n\nstep " << i << " "; ++i;
	tree.disp_board();

	if (color == 'b' || color == 'B') {
		for (; i < 10000; ++i) {
			// human move
			cout << "input x y (negative to pass): ";
			cin >> x >> y;
			if (x < 0 || y < 0) {
				if (tree.pass()) // pass !
					break; // double pass !
			}
			if (tree.place(x, y)) {
				error("illegal move!");
			}
			cout << "step " << i << " ";
			tree.disp_board();

			// computer move
			ret = tree.rand_smart_move();
			if (ret == -1)
				error("unkown error!");
			if (ret == -2) {
				cout << "double pass !\n\n\n";
				break;
			}
			cout << "step " << i << " ";
			tree.disp_board();
		}
	}
	else if (color == 'w' || color == 'W') {
		for (; i < 10000; ++i) {
			// computer move
			ret = tree.rand_smart_move();
			if (ret == -1)
				error("unkown error!");
			if (ret == -2) {
				cout << "double pass !\n\n\n";
				break;
			}
			cout << "step " << i << " ";
			tree.disp_board();

			// human move
			cout << "input x y (negative to pass): ";
			cin >> x >> y;
			if (x < 0 || y < 0) {
				if (tree.pass()) // pass !
					break; // double pass !
			}
			if (tree.place(x, y)) {
				error("illegal move!");
			}
			cout << "step " << i << " ";
			tree.disp_board();
		}
	}
	else
		error("illegal color! must be 'b' or 'w'!");

	cout << "game over!" << "\n\n";

	tree.solve_end();

	if (tree.winner() == Who::BLACK) {
		cout << "black wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else if (tree.winner() == Who::WHITE) {
		cout << "white wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else { // draw
		cout << "draw!\n\n";
	}
	tree.writeSGF("record.sgf");

	getchar();
	getchar();
}

// run this for stand alone human vs computer
void human_vs_human_ui()
{
	Int Nx, Ny, ret, i = 0, bscore4, x, y;
	Doub komi;
	Char color;
	cout << "human vs computer..." << endl;
	cout << "input board size x : "; cin >> Nx;
	cout << "input board size y : "; cin >> Ny;
	cout << "input komi : "; cin >> komi;

	board_Nx(Nx); board_Ny(Ny); // set board size
	komi2(round(komi * 2)); // set koomi
	Tree tree;

	i = 0;
	cout << "\n\nstep " << i << " "; ++i;
	tree.disp_board();

	for (; i < 10000; ++i) {
		// human move
		cout << "input x y (negative to pass): ";
		cin >> x >> y;
		if (x < 0 || y < 0) {
			if (tree.pass()) // pass !
				break; // double pass !
		}
		if (tree.place(x, y)) {
			error("illegal move!");
		}
		cout << "step " << i << " ";
		tree.disp_board();
	}

	cout << "game over!" << "\n\n";
	
	tree.solve_end();

	if (tree.winner() == Who::BLACK) {
		cout << "black wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else if (tree.winner() == Who::WHITE) {
		cout << "white wins!";
		cout << "  (score: " << 0.5*tree.score2() << ")\n\n";
	}
	else { // draw
		cout << "draw!\n\n";
	}
	tree.writeSGF("record.sgf");

	getchar();
	getchar();
}

int main()
{
	// computer_vs_computer_ui();
	// human_vs_computer_ui();
	// human_vs_human_ui();

	board_Nx(3); board_Ny(3); // set board size
	komi2(17); // set koomi
	Tree tree;

	// debug: edit board here
	tree.place(1, 1);
	// end edit board

	tree.solve();
}
