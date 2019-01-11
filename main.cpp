#include <vector>
#include "SLISC/slisc.h"
using namespace slisc;
using std::vector;

enum Who { NONE, BLACK, WHITE, UNKNOWN, DRAW }; // TODO: make this a char type

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
	Move(char x, char y) : m_x(x), m_y(y)
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
	vector<Node*> m_next; // nullptr: last node
	Who m_turn; // who's turn is this?
	Who m_win; // if two gods playing, who will win?
	int m_step; // current step number

	Node() {}
	
	void place(int step, char x, char y, Who turn, Node* last)
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
public:
	Board(char Nx, char Ny) : m_Nx(Nx), m_Ny(Ny), m_data(Nx, Ny) {}
	void place(char x, char y, Who s)
	{
		// check if already occupied
		if (m_data[x][y] != NONE)
			error("Board::place(): occupied!");
		// TODO: check other illegal cases

		// place stone
		m_data[x][y] = s;
	}
};

// game tree
class Tree
{
public:
	size_t ind; // index of this step (already played) in m_data
	vector<Node> m_data;
	Board board;

	Tree(char Nx, char Ny): board(Nx, Ny) {} // specify board size

	size_t step() { return m_data[ind].m_step; } // this step number 

	Who turn() { return m_data[ind].m_turn; } // who played this step

	Who turn1()
	{
		if (m_data[ind].m_turn == BLACK) return WHITE;
		if (m_data[ind].m_turn == WHITE) return BLACK;
		error("Tree::turn1(): ???!");
	}

	void pass()
	{
		Node node;
		node.pass(m_data[ind].m_step + 1, &m_data[ind]);
		m_data.push_back(node);
		++ind;
	}

	void place(char x, char y)
	{
		// update board
		board.place(x, y, turn1());
		// add Node to tree
		Node node;
		node.place(step(), x, y, turn1(), &m_data[ind]);
		++ind;
	}
	
	~Tree() {}
};

int main()
{

}
