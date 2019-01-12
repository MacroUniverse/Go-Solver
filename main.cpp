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
	Long m_pool_ind; // pool index, board stored in Pool::m_board[m_pool_ind]
	// Int m_step; // no step number due to upward branching

	Node() {}
	
	void set_last(Long_I ind) { m_last.push_back(ind); }

	void set_next(Long_I ind) { m_next.push_back(ind); }

	Bool coord(Char_O &x, Char_O &y) { x = m_mov.m_x; y = m_mov.m_y; return m_mov.ispass(); }

	void place(Char_I x, Char_I y, Who_I turn, Long_I last)
	{ m_mov.m_x = x; m_mov.m_y = y; m_turn = turn; m_last.push_back(last); }

	void pass(Who_I turn, Long_I last)
	{ m_mov.m_x = 0; m_mov.m_y = -1; m_turn = turn;  m_last.push_back(last);	}

	~Node() {}
};

// board size Nx, may only set once
inline Char board_Nx(Char_I Nx = -1)
{
	static Char Nx0 = -1;
	if (Nx0 < 0) {
		if (Nx > 0)
			Nx0 = Nx;
		else
			error("board_Nx(): size must be positive!");
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

// status of the board
// TODO: implement operator << for move
class Board
{
public:
	Matrix<Who> m_data;

	// TODO: can these be non member ?
	MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	vector<Move> group; // a group of connected stones
	Int qi; // qi of group

	Board() {}

	Board(Char_I Nx, Char_I Ny) : m_data(Nx, Ny), m_mark(Nx, Ny)
	{ m_data = NONE; m_mark = 0; }

	inline Bool operator==(const Board &rhs)
	{ return this->m_data == rhs.m_data; }

	inline Bool operator<(const Board &rhs); // comparation for sorting

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

// imagine Board::m_data as a long integer, try two compare two integers
inline Bool Board::operator<(const Board &rhs)
{
	Int i, N = board_Nx() * board_Ny();
	for (i = 0; i < N; ++i) {
		if (this->m_data(i) == rhs.m_data(i)) continue;
		if (this->m_data(i) < rhs.m_data(i)) return true;
		return false;
	}
	return false;
}

inline void Board::disp()
{
	Char i, x, y, Nx = board_Nx(), Ny = board_Ny();
	cout << "     ";
	for (i = 0; i < board_Nx(); ++i) cout << Int(i) << "   "; cout << "\n";
	cout << "    ";
	for (x = 0; x < Nx; ++x) cout << "----"; cout << "\n";
	for (y = 0; y < Ny; ++y) {
		cout << " " << Int(y) << " |";
		for (x = 0; x < Nx; ++x) {
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
		for (i = 1; i <= Nx; ++i) cout << "----"; cout << "\n";
	}
}

// return 0 if legal, and update board
// Ko is not considered! 
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Board::place(Char_I x, Char_I y, Who_I s)
{
	Int Nx = board_Nx(), Ny = board_Ny();
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

	if (x < Nx - 1 && m_data(x + 1, y) == inv(s)) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) remove_group();
	}

	if (y > 0 && m_data(x, y - 1) == inv(s)) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) remove_group();
	}

	if (y < Ny && m_data(x, y + 1) == inv(s)) {
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

	if (x < Nx - 1 && m_data(x + 1, y) == s) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) return -2;
	}

	if (y > 0 && m_data(x, y - 1) == s) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) return -2;
	}

	if (y < Ny && m_data(x, y + 1) == s) {
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
	Char Nx = board_Nx(), Ny = board_Ny();
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

	if (x < Nx - 1) {
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

	if (y < Ny - 1) {
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
	Char x, y, Nx = board_Nx(), Ny = board_Ny();
	Int black = 0, qi = 0, common_qi = 0;
	Who s;
	for (x = 0; x < Nx; ++x)
		for (y = 0; y < Ny; ++y) {
			s = m_data(x, y);
			if (s == NONE) {
				// qi not surrounded by black
				if ((x > 0 && m_data(x - 1, y) != BLACK)
					|| (x < Nx - 1 && m_data(x + 1, y) != BLACK)
					|| (y > 0 && m_data(x, y - 1) != BLACK)
					|| (y < Ny - 1 && m_data(x, y + 1) != BLACK)) {
					// qi not surrounded by white
					if ((x > 0 && m_data(x - 1, y) != WHITE)
						|| (x < Nx - 1 && m_data(x + 1, y) != WHITE)
						|| (y > 0 && m_data(x, y - 1) != WHITE)
						|| (y < Ny - 1 && m_data(x, y + 1) != WHITE))
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
// index to Pool::m_data is called pool index, index to Tree::m_data is called tree index, these indices should never change
class Pool
{
public:
	vector<Board> m_boards; // should be the same order as in the Tree, for "pass", store an empty board
	vector<Long> m_treeInd;  // m_treeInd[i] is the index of m_data[i] in the game tree
	vector<Long> m_order; // m_data[m_order[i]] is in sorted order

	Pool() {}

	// get a board by index
	inline Board & get(Long_I ind) { return m_boards[m_order[ind]]; }

	// push board to m_data and insert ind to m_inds
	inline void push(const Board &board, Long_I ind);

	// search Pool: get[search(board)] will match board
	inline Long search(const Board &board);

	// place a stone
	inline Int place(Long_I ind, Char_I x, Char_I y);
};

inline void Pool::push(const Board &board, Long_I ind)
{

}

// return -1 if does not exist
inline Long Pool::search(const Board &board)
{
	Int i, N = board_Nx() * board_Ny();
	for (i = 0; i < N; ++i) {

	}
}

inline Int place(Long_I tree_ind, Char_I x, Char_I y)
{
	// return 0 if legal, and update board
	// Ko is not considered!
	// return -1 if occupied, do nothing
	// return -2 if no qi, do nothing
	// inline Int Board::place(Char_I x, Char_I y, Who_I s)
}

// return 0 if a new board is added
// return index to the same board if "Ko" happens
// return -1 if move is illegal
inline Int place(Long_I ind, Char_I x, Char_I y)
{

}

// game tree
// TODO: make sure no two nodes have the same situation
class Tree
{
public:
	Long m_ind; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_nodes;
	Pool m_pool;

	Tree(): m_ind(-1) {} // specify board size

	Board & board(Long_I ind) { return m_pool.m_boards[ind]; } // reference to the Board obj of a node

	inline Long last(); // index to node of last step (-1 if doesn't exist)

	// inline Int step(); // step is ambiguous due to upward branching

	inline Who turn() { return m_nodes[m_ind].m_turn; } // who played the current step

	inline Bool coord(Char_O &x, Char_O &y, Long_I ind) { return m_nodes[ind].coord(x, y); }

	inline void disp_board(Long_I ind = -1); // display board

	inline Int pass();

	inline Int place(Char_I x, Char_I y);

	inline Int islinked(Long_I ind1, Long_I ind2); // check if node ind1 can lead to node ind2

	inline Int check_ko(); // check if same board already exists in the Pool and decide if it is a Ko
	
	inline void branch(vector<Long> &br, Long_I ind); // get index vector of the a branch ending at a node

	inline void writeSGF(const string &name);

	inline Int rand(Long_I ind = -1); // generate a new random move for a node

	inline void winner(Long_I node); // analyse who has winning strategy for a node

	inline Int score_x2(Long_I ind) { return board(ind).score_x2(); }

	~Tree() {}
};

// if multiple paths exists choose the first one for now
inline Long Tree::last()
{
	if (m_ind == -1) return -1;
	return m_nodes[m_ind].m_last[0];
}

// ind = m_ind my default
inline void Tree::disp_board(Long_I ind)
{
	Char x, y;
	Long ind1 = ind < 0 ? m_ind : ind;
	if (ind1 < 0)
		cout << "(empty)";
	else {
		if (turn() == BLACK)
			cout << "(black ";
		else if (turn() == WHITE)
			cout << "(white ";
		else
			error("Tree:disp(): illegal side name!");

		if (coord(x, y, ind1))
			cout << "pass)";
		else
			cout << "[" << Int(x) << "," << Int(y) << "] )";
	}
	
	cout << "\n\n";
	board(ind).disp();
	cout << '\n' << endl;
};

// return -1 if game ends, 0 if not
inline Int Tree::pass()
{
	Node node;
	node.pass(inv(turn()), m_ind);
	m_nodes.push_back(node);
	++m_ind;
	// check double pass (game ends)
	if (m_nodes[m_ind - 1].m_mov.ispass()) {
		m_nodes[m_ind].m_next.push_back(-1);
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
		node.place(x, y, BLACK, -1);
		m_nodes.push_back(node);
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
	node.place(x, y, inv(turn()), m_ind);
	m_nodes.push_back(node);
	m_nodes[m_ind].set_next(m_ind + 1);
	++m_ind;
}

// return 0 if not linked
// return 1 if node ind1 can lead to node ind2
// upward search from node ind2 is most efficient
// this is a recursive function to deal with upward branching
inline Int Tree::islinked(Long_I ind1, Long_I ind2)
{
	Long i, ind = ind2;
	for (i = 0; i < 10000; ++i) {
		if (m_nodes[ind].m_last.size() > 1) break; // multiple upward branch
		if (ind == ind2) return 1; // found ind2
		if (m_nodes[ind].m_last[0] == -1) return 0; // reached top of tree
		ind = m_nodes[ind].m_last[0]; // single line, go up
	}
	for (i = 0; i < m_nodes[ind].m_last.size(); ++i) {
		if (islinked(ind1, m_nodes[ind].m_last[i]) == 1)
			return 1;
	}
}

// if multiple paths exists choose the first one for now
inline void Tree::branch(vector<Long> &br, Long_I ind)
{
	Int i, Nbr;
	Long ind1 = m_ind;
	br.resize(0);
	for (i = 0; i < 10000; ++i) {
		br.push_back(ind1);
		ind1 = m_nodes[ind1].m_last[0];
		if (ind1 == -1) break;
	}
	Nbr = br.size();
	for (i = Nbr / 2; i > -1; --i) {
		SWAP(br[Nbr - i - 1], br[i]);
	}
}

inline void Tree::writeSGF(const string &name)
{
	Char Nx = board_Nx(), Ny = board_Ny();
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[0]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Nx << ":" << Ny << "]";
	fout << "DT[]\n";

	// write current branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Long> br;
	branch(br, m_ind);
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
	Int i, j, Nx = size_x(), Ny = size_y(), Nxy = Nx*Ny;
	Char x0, y0, x, y;
	VecInt xy;
	vector<Long> & next = m_nodes[ind].m_next;

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
	Tree tree(3,3); tree.disp_board();
	cout << "black score: " << tree.score_x2()/2. << "\n\n";
	
	for (i = 0; i < 100; ++i) {
		if (tree.rand() != 0) break;
		tree.disp_board();
		cout << "black score: " << tree.score_x2() / 2. << "\n\n";
	}
	
	cout << "game over!" << "\n\n";
	tree.writeSGF("test.sgf");
}
