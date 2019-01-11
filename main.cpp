#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "SLISC/slisc.h"

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
	Int m_last; // -1: first node
	vector<Int> m_next; // -1: end node
	Who m_turn; // who's turn is this?
	Who m_win; // if two gods playing, who will win?
	Int m_step; // current step number

	Node() {}
	
	void set_last(Int_I ind) { m_last = ind; }

	void set_next(Int_I ind) { m_next.push_back(ind); }

	Bool coord(Char_O &x, Char_O &y) { x = m_mov.m_x; y = m_mov.m_y; return m_mov.ispass(); }

	void place(Int step, Char_I x, Char_I y, Who_I turn, Int_I last)
	{ m_step = step; m_mov.m_x = x; m_mov.m_y = y; m_turn = turn; m_last = last; }

	void pass(Int step, Who_I turn, Int_I last)
	{ m_step = step; m_mov.m_x = 0; m_mov.m_y = -1; m_turn = turn;  m_last = last;	}

	~Node() {}
};


// status of the board
class Board
{
public:
	char m_Nx, m_Ny;
	Matrix<Who> m_data;
	MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	vector<Move> group; // a group of connected stones
	Int qi; // qi of group

	Board(Char_I Nx, Char_I Ny) : m_Nx(Nx), m_Ny(Ny), m_data(Nx, Ny), m_mark(Nx, Ny)
	{ m_data = NONE; m_mark = 0; }

	inline void disp(); // display board on screen
	inline void place(Char_I x, Char_I y, Who_I s);

	inline void connect_init(); // use before "connect()"

	// get stones connected to a given stone (fill "m_group") and calculate qi
	inline void connect(Char_I x, Char_I y);

	// remove group if it is dead after placing (x,y)
	inline void remove_group();

	~Board() {}
};

inline void Board::disp()
{
	Int i, j;
	cout << "  ";
	for (i = 1; i <= m_Nx; ++i) cout << i << ' '; cout << "\n";
	cout << "  ";
	for (i = 1; i <= m_Nx; ++i) cout << "--"; cout << "\n";
	for (j = 0; j < m_Ny; ++j) {
		cout << j + 1 << '|';
		for (i = 0; i < m_Nx; ++i) {
			if (m_data[i][j] == NONE)
				cout << " |";
			else if (m_data[i][j] == BLACK)
				cout << "@|";
			else if (m_data[i][j] == WHITE)
				cout << "O|";
			else
				error("Board::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "  ";
		for (i = 1; i <= m_Nx; ++i) cout << "--"; cout << "\n";
	}
}

inline void Board::place(Char_I x, Char_I y, Who_I s)
{
	// check if already occupied
	if (m_data[x][y] != NONE)
		error("Board::place(): occupied!");
	// TODO: check other illegal cases

	// place stone
	m_data[x][y] = s;

	// === remove dead stones ===

	// if placed next to opposite stone
	if (x > 0 && m_data[x - 1][y] == inv(s)) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) remove_group();
	}

	if (x < m_Nx - 1 && m_data[x + 1][y] == inv(s)) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) remove_group();
	}

	if (y > 0 && m_data[x][y - 1] == inv(s)) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) remove_group();
	}

	if (y < m_Ny && m_data[x][y + 1] == inv(s)) {
		connect_init();
		connect(x, y + 1);
		if (qi == 0) remove_group();
	}

	// if placed next to same color stone

	if (x > 0 && m_data[x - 1][y] == s) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) error("Board::place(): illegal move!");
	}

	if (x < m_Nx - 1 && m_data[x + 1][y] == s) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) error("Board::place(): illegal move!");
	}

	if (y > 0 && m_data[x][y - 1] == s) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) error("Board::place(): illegal move!");
	}

	if (y < m_Ny && m_data[x][y + 1] == s) {
		connect_init();
		connect(x, y + 1);
		if (qi == 0) error("Board::place(): illegal move!");
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
	Who s, s0 = m_data[x][y];
	group.push_back(Move(x, y));
	m_mark[x][y] = 1;

	if (x > 0) {
		s = m_data[x - 1][y];
		Char & mark = m_mark[x - 1][y];
		if (s == s0 && !mark)
			connect(x - 1, y);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (x < m_Nx - 1) {
		s = m_data[x + 1][y];
		Char & mark = m_mark[x + 1][y];
		if (s == s0 && !mark)
			connect(x + 1, y);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (y > 0) {
		s = m_data[x][y - 1];
		Char & mark = m_mark[x][y - 1];
		if (s == s0 && !mark)
			connect(x, y - 1);
		else if (s == NONE && !mark) {
			++qi; mark = 1;
		}
	}

	if (y < m_Ny - 1) {
		s = m_data[x][y + 1];
		Char & mark = m_mark[x][y + 1];
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
		m_data[group[i].m_x][group[i].m_y] = NONE;
}

// game tree
class Tree
{
public:
	Int m_ind; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_data;
	Board board;

	Tree(Char_I Nx, Char_I Ny): m_ind(-1), board(Nx, Ny) {} // specify board size

	inline Int last(); // index to node of last step (-1 if doesn't exist)

	inline Int step(); // current step number

	inline Who turn() { return m_data[m_ind].m_turn; } // who played the current step

	inline Bool coord(Char_O &x, Char_O &y, Int_I ind) { return m_data[ind].coord(x, y); }

	inline void disp(); // display board

	inline void pass();

	inline void place(Char_I x, Char_I y);
	
	inline void branch(vector<Int> &br); // get index vector of the current branch

	inline void writeSGF(const string &name);

	~Tree() {}
};

inline Int Tree::last()
{
	if (m_ind == -1) return -1;
	return m_data[m_ind].m_last;
}

inline Int Tree::step()
{
	if (m_ind == -1) return -1;
	return m_data[m_ind].m_step;
}

inline void Tree::disp()
{
	cout << "step: " << step() << "\n\n";
	board.disp();
	cout << '\n' << endl;
};

inline void Tree::pass()
{
	Node node;
	node.pass(step() + 1, inv(turn()), m_ind);
	m_data.push_back(node);
	++m_ind;
}

inline void Tree::place(Char_I x, Char_I y)
{
	if (m_ind == -1) { // first move
		board.place(x, y, BLACK);
		Node node;
		node.place(0, x, y, BLACK, -1);
		m_data.push_back(node);
		++m_ind;
		return;
	}

	// update board
	board.place(x, y, inv(turn()));
	// add Node to tree
	Node node;
	node.place(step() + 1, x, y, inv(turn()), m_ind);
	m_data.push_back(node);
	m_data[m_ind].set_next(m_ind + 1);
	++m_ind;
}

inline void Tree::branch(vector<Int> &br)
{
	if (step() < 0) { br.resize(0);  return; }
	Int i;
	br.resize(step() + 1); // node index
	br[step()] = m_ind;
	for (i = step(); i > 0; --i)
		br[i - 1] = m_data[br[i]].m_last;
}

inline void Tree::writeSGF(const string &name)
{
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[0]";

	// board size
	if (board.m_Nx == board.m_Ny)
		fout << "SZ[" << Int(board.m_Nx) << "]";
	else
		fout << "SZ[" << board.m_Nx << ":" << board.m_Ny << "]";
	fout << "DT[]\n";

	// write current branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Int> br;
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


int main()
{
	Tree tree(3,3); tree.disp();
	
	tree.place(1, 1); tree.disp();
	
	tree.place(1, 0); tree.disp();
	
	tree.place(1, 2); tree.disp();
	
	tree.place(0, 1); tree.disp();
	
	tree.place(2, 1); tree.disp();
	
	tree.place(0, 0); tree.disp();
	
	tree.place(0, 2); tree.disp();
	
	tree.pass(); tree.disp();
	
	tree.place(2, 0); tree.disp();
	
	tree.writeSGF("test.sgf");
}
