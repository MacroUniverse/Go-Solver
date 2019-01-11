#include <vector>
#include "SLISC/slisc.h"
using namespace slisc;
using std::vector;

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
class Move
{	
public:
	// origin at top left, x axis points right, y axis points down
	char m_x, m_y;
	// (0,0): undefined
	// (0,-1): pass
	// others: illegal

	Move() : m_x(0), m_y(0) {}
	Move(Char_I x, Char_I y) : m_x(x), m_y(y)
	{
		if (x < 0 || y < 0)
			error("Move::Move(x, y): coord < 0 !");
	}

	void pass() { m_x = 0; m_y = -1; }
	~Move() {}
};

// a node in the game tree
class Node
{
public:
	Move m_mov;
	Node* m_last; // nullptr: first node
	vector<Node*> m_next; // nullptr: end node
	Who m_turn; // who's turn is this?
	Who m_win; // if two gods playing, who will win?
	int m_step; // current step number

	Node() {}
	
	void set_last(Node *last) { m_last = last; }

	void set_next(Node *next) { m_next.push_back(next); }


	void place(int step, Char_I x, Char_I y, Who_I turn, Node* last)
	{
		m_step = step; m_mov.m_x = x; m_mov.m_y = y; m_turn = turn; m_last = last;
	}

	void pass(int step, Node *last) { m_step = step; m_last = last; }

	~Node() {}
};


// status of the board
class Board
{
private:
	char m_Nx, m_Ny;
	Matrix<Who> m_data;
	MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	vector<Move> group; // a group of connected stones
	Int qi; // qi of group
public:
	Board(Char_I Nx, Char_I Ny) : m_Nx(Nx), m_Ny(Ny), m_data(Nx, Ny) { m_data = NONE; m_mark = 0; }

	void place(Char_I x, Char_I y, Who_I s)
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

	// use this before "connect()"
	void connect_init()
	{
		m_mark = 0;
		group.resize(0);
		qi = 0;
	}

	// get stones connected to a given stone (fill "m_group")
	// and calculate qi
	void connect(Char_I x, Char_I y)
	{
		group.push_back(Move(x, y));
		m_mark[x][y] = 1;
		
		if (x > 0)
			if (m_data[x - 1][y] != NONE)
				connect(x - 1, y);
			else ++qi;
		
		if (x < m_Nx - 1)
			if (m_data[x + 1][y] != NONE)
				connect(x + 1, y);
			else ++qi;

		if (y > 0)
			if (m_data[x][y - 1] != NONE)
				connect(x, y - 1);
			else ++qi;

		if (y < m_Ny - 1)
			if (m_data[x][y + 1] != NONE)
				connect(x, y + 1);
			else ++qi;
	}

	// remove group if it is dead after placing (x,y)
	void remove_group()
	{
		Int i;
		for (i = 0; i < group.size(); ++i)
			m_data[group[i].m_x][group[i].m_y] = NONE;
	}
};

// game tree
class Tree
{
public:
	size_t ind; // index of the current step (already played) in m_data
	vector<Node> m_data;
	Board board;

	Tree(Char_I Nx, Char_I Ny): ind(-1), board(Nx, Ny) {} // specify board size

	Node * ptr() { return &m_data[ind]; } // pointer to current node

	size_t step() { return m_data[ind].m_step; } // current step number 

	Who turn() { return m_data[ind].m_turn; } // who played the current step

	Who turn1()
	{
		if (m_data[ind].m_turn == BLACK) return WHITE;
		if (m_data[ind].m_turn == WHITE) return BLACK;
		error("Tree::turn1(): ???!");
	}

	void pass()
	{
		Node node;
		node.pass(m_data[ind].m_step + 1, ptr());
		m_data.push_back(node);
		++ind;
	}

	void place(Char_I x, Char_I y)
	{
		if (ind == -1) { // first move
			Node node;
			node.place(0, x, y, BLACK, nullptr);
			m_data.push_back(node);
			++ind;
		}

		// update board
		board.place(x, y, turn1());
		// add Node to tree
		Node node;
		node.place(step(), x, y, turn1(), ptr());
		m_data.push_back(node);
		m_data[ind].set_next(&m_data.back());
		++ind;
	}
	
	~Tree() {}
};

int main()
{
	Tree tree(3,3);
	tree.place(1, 1);
	tree.place(1, 0);
	tree.place(1, 2);
	tree.place(0, 1);
}
