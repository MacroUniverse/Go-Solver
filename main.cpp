#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "SLISC/slisc.h"
#include "SLISC/random.h"

using namespace slisc;
using std::vector; using std::string;
using std::ofstream; using std::cout; using std::endl;

enum Who { NONE, BLACK, WHITE, UNKNOWN, DRAW }; // TODO: make this a char type

typedef const Who Who_I;

// inverse color
inline Who inv(Who_I s)
{
	if (s == BLACK) return WHITE;
	if (s == WHITE) return BLACK;
	error("opposite(Who_I): unknown error!");
}

// coordinates for a move, or "pass"
// origin at top left, x axis points right, y axis points down
// (0,0): undefined
// (0,-1): pass
// others: illegal

class Move
{	
public:
	char m_x, m_y;
	Move() : m_x(0), m_y(0) {}
	inline Move(Char_I x, Char_I y);
	void pass() { m_x = 0; m_y = -1; }
	inline Bool ispass();
	~Move() {}
};

inline Move::Move(Char_I x, Char_I y) : m_x(x), m_y(y)
{
	if (x < 0 || y < 0)
		error("Move::Move(x, y): coord < 0 !");
}

inline Bool Move::ispass()
{
	if (m_x == 0 && m_y == -1)
		return true;
	else if (m_x < 0 || m_y < 0)
		error("Move::ispass(x, y): illegal coordinates!");
	else
		return false;
}

// a node in the game tree
class Node
{
public:
	Move m_mov;
	vector<Long> m_last; // -1: first node
	vector<Long> m_next; // -1: end node
	Who m_turn; // who's turn is this?
	Who m_win; // if two gods playing, who will win?
	Int m_step; // current step number

	Node() {}
	
	void set_last(Long_I ind) { m_last.push_back(ind); }

	void set_next(Long_I ind) { m_next.push_back(ind); }

	Bool coord(Char_O &x, Char_O &y) { x = m_mov.m_x; y = m_mov.m_y; return m_mov.ispass(); }

	void place(Int_I step, Char_I x, Char_I y, Who_I turn, Long_I last)
	{ m_step = step; m_mov.m_x = x; m_mov.m_y = y; m_turn = turn; m_last.push_back(last); }

	void pass(Int_I step, Who_I turn, Long_I last)
	{ m_step = step; m_mov.m_x = 0; m_mov.m_y = -1; m_turn = turn;  m_last.push_back(last);	}

	~Node() {}
};


// status of the board
class Board
{
public:
	char m_Nx, m_Ny;
	Matrix<Who> m_data;

	// TODO: can these be non member ?
	MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	vector<Move> group; // a group of connected stones
	Int qi; // qi of group

	Board(Char_I Nx, Char_I Ny) : m_Nx(Nx), m_Ny(Ny), m_data(Nx, Ny), m_mark(Nx, Ny)
	{ m_data = NONE; m_mark = 0; }

	inline void disp(); // display board on screen

	inline Int place(Char_I x, Char_I y, Who_I s); // place stone

	inline void connect_init(); // use before "connect()"

	// get stones connected to a given stone (fill "m_group") and calculate qi
	inline void connect(Char_I x, Char_I y);

	// remove group if it is dead after placing (x,y)
	inline void remove_group();

	inline Int score_x2(); // count black result (multiplied by 2)

	~Board() {}
};

inline void Board::disp()
{
	Char i, x, y;
	cout << "     ";
	for (i = 0; i < m_Nx; ++i) cout << Int(i) << "   "; cout << "\n";
	cout << "    ";
	for (x = 0; x < m_Nx; ++x) cout << "----"; cout << "\n";
	for (y = 0; y < m_Ny; ++y) {
		cout << " " << Int(y) << " |";
		for (x = 0; x < m_Nx; ++x) {
			if (m_data(x,y) == NONE)
				cout << "   |";
			else if (m_data(x,y) == BLACK)
				cout << " @ |";
			else if (m_data(x,y) == WHITE)
				cout << " O |";
			else
				error("Board::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "    ";
		for (i = 1; i <= m_Nx; ++i) cout << "----"; cout << "\n";
	}
}

// return 0 if legal, and update board
// Ko is not considered! 
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Board::place(Char_I x, Char_I y, Who_I s)
{
	// check if already occupied
	if (m_data(x,y) != NONE)
		return -1;
	// TODO: check other illegal cases

	// place stone
	m_data(x,y) = s;

	// === remove dead stones ===

	// if placed next to opposite stone
	if (x > 0 && m_data(x - 1, y) == inv(s)) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) remove_group();
	}

	if (x < m_Nx - 1 && m_data(x + 1, y) == inv(s)) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) remove_group();
	}

	if (y > 0 && m_data(x, y - 1) == inv(s)) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) remove_group();
	}

	if (y < m_Ny && m_data(x, y + 1) == inv(s)) {
		connect_init();
		connect(x, y + 1);
		if (qi == 0) remove_group();
	}

	// if placed next to same color stone

	if (x > 0 && m_data(x - 1, y) == s) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) return -2;
	}

	if (x < m_Nx - 1 && m_data(x + 1, y) == s) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) return -2;
	}

	if (y > 0 && m_data(x, y - 1) == s) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) return -2;
	}

	if (y < m_Ny && m_data(x, y + 1) == s) {
		connect_init();
		connect(x, y + 1);
		if (qi == 0) return -2;
	}
}

inline void Board::connect_init()
{
	m_mark = 0;
	group.resize(0);
	qi = 0;
}

inline void Board::connect(Char_I x, Char_I y)
{
	Who s, s0 = m_data(x, y);
	group.push_back(Move(x, y));
	m_mark(x, y) = 1;

	if (x > 0) {
		s = m_data(x - 1, y);
		Char & mark = m_mark(x - 1, y);
		if (s == s0 && !mark)
			connect(x - 1, y);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (x < m_Nx - 1) {
		s = m_data(x + 1, y);
		Char & mark = m_mark(x + 1, y);
		if (s == s0 && !mark)
			connect(x + 1, y);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (y > 0) {
		s = m_data(x, y - 1);
		Char & mark = m_mark(x, y - 1);
		if (s == s0 && !mark)
			connect(x, y - 1);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (y < m_Ny - 1) {
		s = m_data(x, y + 1);
		Char & mark = m_mark(x, y + 1);
		if (s == s0 && !mark)
			connect(x, y + 1);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}
}

inline void Board::remove_group()
{
	Int i;
	for (i = 0; i < group.size(); ++i)
		m_data(group[i].m_x, group[i].m_y) = NONE;
}

// this will be accurate when no legal move exists
// back score = # of black on board + single "qi" surrounded by black + (other empty space)/2
inline Int Board::score_x2()
{
	Char x, y;
	Int black = 0, qi = 0, common_qi = 0;
	Who s;
	for (x = 0; x < m_Nx; ++x)
		for (y = 0; y < m_Ny; ++y) {
			s = m_data(x, y);
			if (s == NONE) {
				// qi not surrounded by black
				if ((x > 0 && m_data(x - 1, y) != BLACK)
					|| (x < m_Nx - 1 && m_data(x + 1, y) != BLACK)
					|| (y > 0 && m_data(x, y - 1) != BLACK)
					|| (y < m_Ny - 1 && m_data(x, y + 1) != BLACK)) {
					// qi not surrounded by white
					if ((x > 0 && m_data(x - 1, y) != WHITE)
						|| (x < m_Nx - 1 && m_data(x + 1, y) != WHITE)
						|| (y > 0 && m_data(x, y - 1) != WHITE)
						|| (y < m_Ny - 1 && m_data(x, y + 1) != WHITE))
						++common_qi;
				}
				else // qi surrounded by black
					++qi;
			}
			else if (s == BLACK)
				++black;
			else if (s != WHITE)
				error("Board::result(): illegal stone!");
		}
	return 2 * black + 2 * qi + common_qi;
}

// all situations in the tree, sorted for quick search
// sorting: each board is a radix 3 number, sort these numbers with ascending order
class Pool
{
public:
	vector<Board> m_data; // should be the same order as in the Tree
	vector<Long> m_treeInd;  // m_treeInd[i] is the index of m_data[i] in the game tree
	vector<Long> m_order; // m_data[m_order[i]] is in sorted order

	// get a board by index
	inline Board & get(Long_I ind) { return m_data[m_order[ind]]; }

	// push board to m_data and insert ind to m_inds
	inline void push(const Board &board, Long_I ind);

	// search Pool: get[search(board)] will match board
	inline Long search(const Board &board);

};

inline void Pool::push(const Board &board, Long_I ind)
{

}

// return -1 if does not exist
inline Long Pool::search(const Board &board)
{

}

// game tree
// TODO: make sure no two nodes have the same situation
class Tree
{
public:
	Long m_ind; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_data;
	Board m_board;

	Tree(Char_I Nx, Char_I Ny): m_ind(-1), m_board(Nx, Ny) {} // specify board size

	inline Long last(); // index to node of last step (-1 if doesn't exist)

	inline Int step(); // current step number

	inline Who turn() { return m_data[m_ind].m_turn; } // who played the current step

	inline Bool coord(Char_O &x, Char_O &y, Long_I ind) { return m_data[ind].coord(x, y); }

	inline void disp(); // display board

	inline Int pass();

	inline Int place(Char_I x, Char_I y);
	
	inline void branch(vector<Long> &br); // get index vector of the current branch

	inline void writeSGF(const string &name);

	inline Int rand(Long_I ind = -1); // generate a new random move for a node

	inline void winner(Long_I node); // analyse who has winning strategy for a node

	inline Int score_x2() { return m_board.score_x2(); }

	~Tree() {}
};

// if multiple paths exists choose the first one for now
inline Long Tree::last()
{
	if (m_ind == -1) return -1;
	return m_data[m_ind].m_last[0];
}

inline Int Tree::step()
{
	if (m_ind == -1) return -1;
	return m_data[m_ind].m_step;
}

inline void Tree::disp()
{
	Char x, y;
	cout << "step: " << step() << " ";
	if (m_ind < 0)
		cout << "(empty)";
	else {
		if (turn() == BLACK)
			cout << "(black ";
		else if (turn() == WHITE)
			cout << "(white ";
		else
			error("Tree:disp(): illegal side name!");

		if (coord(x, y, m_ind))
			cout << "pass)";
		else
			cout << "[" << Int(x) << "," << Int(y) << "] )";
	}
	
	cout << "\n\n";
	m_board.disp();
	cout << '\n' << endl;
};

// return -1 if game ends, 0 if not
inline Int Tree::pass()
{
	Node node;
	node.pass(step() + 1, inv(turn()), m_ind);
	m_data.push_back(node);
	++m_ind;
	// check double pass (game ends)
	if (m_data[m_ind - 1].m_mov.ispass()) {
		m_data[m_ind].m_next.push_back(-1);
		return -1;
	}
	return 0;
}

// return 0 if legal, -1 if illegal and show error
inline Int Tree::place(Char_I x, Char_I y)
{
	if (m_ind == -1) { // first move
		m_board.place(x, y, BLACK);
		Node node;
		node.place(0, x, y, BLACK, -1);
		m_data.push_back(node);
		++m_ind;
		return;
	}

	// update board and check illegal move (Ko no checked!)
	if (m_board.place(x, y, inv(turn())) != 0) {
		error("Tree::place(x,y): illegal move!");
		return -1;
	}

	// check Ko

	// add Node to tree
	Node node;
	node.place(step() + 1, x, y, inv(turn()), m_ind);
	m_data.push_back(node);
	m_data[m_ind].set_next(m_ind + 1);
	++m_ind;
}

// if multiple paths exists choose the first one for now
inline void Tree::branch(vector<Long> &br)
{
	if (step() < 0) { br.resize(0);  return; }
	Int i;
	br.resize(step() + 1); // node index
	br[step()] = m_ind;
	for (i = step(); i > 0; --i)
		br[i - 1] = m_data[br[i]].m_last[0];
}

inline void Tree::writeSGF(const string &name)
{
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[0]";

	// board size
	if (m_board.m_Nx == m_board.m_Ny)
		fout << "SZ[" << Int(m_board.m_Nx) << "]";
	else
		fout << "SZ[" << m_board.m_Nx << ":" << m_board.m_Ny << "]";
	fout << "DT[]\n";

	// write current branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Long> br;
	branch(br);
	for (i = 0; i < br.size(); ++i) {
		// black moves
		fout << "  ;" << BW;
		if (coord(x, y, br[i])) // pass
			fout << "[]\n";
		else
			fout << '[' << char('a' + x) << char('a' + y) << "]\n";
		BW = BW == 'B' ? 'W' : 'B';
	}
	fout << ")\n";
	fout.close();
}

// return 0 if successful
// return -1 if there is all legal moves are already in the tree
Int Tree::rand(Long_I ind)
{
	Bool exist, exist_pass = false;
	Int i, j, Nx = m_board.m_Nx, Ny = m_board.m_Ny, Nxy = Nx*Ny;
	Char x0, y0, x, y;
	VecInt xy;
	vector<Long> & next = m_data[ind].m_next;

	// random sequence of all grid points on board
	randPerm(xy, Nxy);

	// search xy for a new legal move
	for (i = 0; i < Nxy; ++i) {		
		x = xy[i] % Nx; y = xy[i] / Nx;
		// check existence
		exist = false;
		for (j = next.size(); j > -1; --j) {
			if (!coord(x0, y0, next[j])) {
				if (x == x0 && y == y0)
					exist = true; break;
			}
			else exist_pass = true;
		}
		if (exist) continue;
		
		if (place(x, y) != 0) continue;
		else return 0;
	}

	// no legal move exists, consider passing
	if (exist_pass) return -1;
	return pass();
}

void Tree::winner(Long_I ind)
{
	// TODO: this might be difficult
}

int main()
{
	Int i;
	Tree tree(3,3); tree.disp();
	cout << "black score: " << tree.score_x2()/2. << "\n\n";
	
	for (i = 0; i < 100; ++i) {
		if (tree.rand() != 0) break;
		tree.disp();
		cout << "black score: " << tree.score_x2() / 2. << "\n\n";
	}
	
	cout << "game over!" << "\n\n";
	tree.writeSGF("test.sgf");
}
