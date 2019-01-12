#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "SLISC/slisc.h"
#include "SLISC/random.h"
#include "SLISC/search.h"

#ifndef NDEBUG
#define _CHECK_BOUND_
#endif

using namespace slisc;
using std::vector; using std::string;
using std::ofstream; using std::cout; using std::endl;

enum Who { NONE, BLACK, WHITE, UNKNOWN, DRAW }; // TODO: make this a char type

enum Act { PLACE, PASS, EDIT, INIT }; // TODO: make this a char type

typedef const Who Who_I;

// inverse color
inline Who inv(Who_I s)
{
	if (s == BLACK) return WHITE;
	if (s == WHITE) return BLACK;
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
	Move() {}
	Move(Char_I x, Char_I y);
	Move(const Act type) // 1: pass(), 2 : edit(), 3 : init()
	{ assert(type != PLACE); m_x = -type; }

	// properties
	Bool isplace() { return m_x >= 0; }
	Bool ispass() { return m_x == -1; }
	Bool isedit() { return m_x == -2; }
	Bool isinit() { return m_x == -3; }
	Int type() // 0: isplace(), 1: ispass(), 2: isedit(), 3: isinit()
	{ return isplace()? 0: ispass()? 1: isedit()? 2: 3; }
	inline Char_O x();
	inline Char_O y();
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

inline Char_O Move::x()
{
#ifdef _CHECK_BOUND_
	if ( m_x < 0 ) error("Move::x(): not a coord!");
#endif
	return m_x;
}

inline Char_O Move::y()
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
	Who m_win; // if two gods playing, who will win?
	Long m_poolInd; // pool index, board stored in Pool::m_board[m_pool_ind]
public:

	Node() {}
	
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
	Long next(Int_I ind) const { return m_next[ind]; }

	// set
	void init() { Move::init(); m_last.resize(0); m_who = NONE; }
	void push_last(Long_I treeInd) { m_last.push_back(treeInd); }
	void push_next(Long_I treeInd) { m_next.push_back(treeInd); }

	void place(Char_I x, Char_I y, Who_I who, Long_I treeInd, Long_I poolInd) // already has bound checking
	{ Move::place(x, y); m_who = who; m_last.push_back(treeInd); m_poolInd = poolInd; }

	void pass(Who_I who, Long_I treeInd, Long_I poolInd)
	{ Move::pass(); m_last.push_back(treeInd); m_poolInd = poolInd; }

	~Node() {}
};

// status of the board
// TODO: implement operator << for move
// TODO: use a better comparation rule so that rotations of a board are considered equal, otherwise unequal
class Board
{
public:
	Matrix<Who> m_data;

	// TODO: can these be non member ?
	MatChar m_mark; // 0: unmarked, 1: marked, else: not used yet
	vector<Move> group; // a group of connected stones
	Int qi; // qi of group

	Board() {}

	Board(Char_I Nx, Char_I Ny): m_data(Nx, Ny), m_mark(Nx, Ny)
	{ m_data = NONE; m_mark = 0; }

	inline Bool operator==(const Board &rhs) const
	{ return this->m_data == rhs.m_data; }

	inline Bool operator<(const Board &rhs) const; // comparation for sorting

	inline void disp() const; // display board on screen

	inline Int place(Char_I x, Char_I y, Who_I s); // place stone

	inline void connect_init(); // use before "connect()"

	// get stones connected to a given stone (fill "m_group") and calculate qi
	inline void connect(Char_I x, Char_I y);

	// remove group if it is dead after placing (x,y)
	inline void remove_group();

	inline Int score_x2() const; // count black result (multiplied by 2)

	~Board() {}
};

// imagine Board::m_data as a long integer, try two compare two integers
inline Bool Board::operator<(const Board &rhs) const
{
	Int i, N = board_Nx() * board_Ny();
	for (i = 0; i < N; ++i) {
		if (this->m_data(i) == rhs.m_data(i)) continue;
		if (this->m_data(i) < rhs.m_data(i)) return true;
		return false;
	}
	return false;
}

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
inline Int Board::place(Char_I x, Char_I y, Who_I who)
{
#ifdef _CHECK_BOUND_
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Tree::place(x,y): out of bound!");
#endif
	Int Nx = board_Nx(), Ny = board_Ny();
	// check if already occupied
	if (m_data(x,y) != NONE)
		return -1;
	// TODO: check other illegal cases

	// place stone
	m_data(x,y) = who;

	// === remove dead stones ===

	// if placed next to opposite stone
	if (x > 0 && m_data(x - 1, y) == inv(who)) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) remove_group();
	}

	if (x < Nx - 1 && m_data(x + 1, y) == inv(who)) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) remove_group();
	}

	if (y > 0 && m_data(x, y - 1) == inv(who)) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) remove_group();
	}

	if (y < Ny && m_data(x, y + 1) == inv(who)) {
		connect_init();
		connect(x, y + 1);
		if (qi == 0) remove_group();
	}

	// if placed next to same color stone

	if (x > 0 && m_data(x - 1, y) == who) {
		connect_init();
		connect(x - 1, y);
		if (qi == 0) return -2;
	}

	if (x < Nx - 1 && m_data(x + 1, y) == who) {
		connect_init();
		connect(x + 1, y);
		if (qi == 0) return -2;
	}

	if (y > 0 && m_data(x, y - 1) == who) {
		connect_init();
		connect(x, y - 1);
		if (qi == 0) return -2;
	}

	if (y < Ny && m_data(x, y + 1) == who) {
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
		m_data(group[i].x(), group[i].y()) = NONE;
}

// this will be accurate when no legal move exists
// back score = # of black on board + single "qi" surrounded by black + (other empty space)/2
inline Int Board::score_x2() const
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
// a pool index (poolInd) is an index for m_boards, this index should never change for the same board
// m_treeInd will link to a tree node that is not a PASS
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
	Int search(Long_O treeInd, Long_O orderInd, const Board &board);

	// add a new board to the Pool
	inline void push(const Board &board, Int_I search_ret, Long_I orderInd, Long_I treeInd);
};

Int Pool::search(Long_O treeInd, Long_O orderInd, const Board &board)
{
	Int ret = lookupInt(orderInd, *this, board);
	treeInd = m_treeInd[m_order[orderInd]];
	return ret;
}

// orderInd is output by search() and search_ret is returned by search()
// orderInd should be -3, -2, -1 or 1
inline void Pool::push(const Board &board, Int_I search_ret, Long_I orderInd, Long_I treeInd)
{
	m_boards.resize(m_boards.size()+1);
	m_boards.back() = board;
	m_treeInd.push_back(treeInd);
	Long poolInd = m_boards.size();
	if (search_ret == -2)
		m_order.insert(m_order.begin() + orderInd + 1, poolInd);
	else if (search_ret == -1)
		m_order.insert(m_order.begin(), poolInd);
	else if (search_ret == 1 || search_ret == -3)
		m_order.push_back(poolInd);
	else
		error("Pool::push(): unknown search_ret!");
}

// return 0 if a new board is added
// return index to the same board if "Ko" happens
// return -1 if move is illegal
inline Int place(Long_I ind, Char_I x, Char_I y)
{

}

// game tree
// a tree index is an index for m_nodes (treeInd), this index should never change for the same node
// if any node has nlast() > 1, it creates an "upward fork"
// TODO: make sure no two nodes have the same situation
// TODO: treeInd = 0 should be the empty board!
class Tree
{
public:
	Long m_treeInd; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_nodes;
	Pool m_pool;

	Tree();

	// peoperties
	const Board & get_board(Long_I treeInd = -1) const // reference to the Board obj of a node
	{ return m_pool.m_boards[treeInd<0 ? m_treeInd : treeInd]; }

	inline Long last(Long_I treeInd = -1, Int_I forkInd = 0) const; // return an index of the last node of a node

	Node & lastNode(Long_I treeInd = -1, Int_I forkInd = 0) // return a reference of the last node of a node
	{ return m_nodes[last(treeInd, forkInd)]; }

	Who who(Long_I treeInd = -1) // who played the current step
	{ return m_nodes[treeInd < -1 ? m_treeInd : treeInd ].who(); }

	inline void disp_board(Long_I treeInd = -1); // display board

	inline Int pass(Long_I treeInd = -1);

	inline Int place(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int islinked(Long_I ind1, Long_I ind2); // check if node ind1 can lead to node ind2

	// check if same board already exists in the Pool and decide if it is a Ko
	inline Int check_ko(Long_O &treeInd1, Long_O &orderInd, Long_I treeInd, const Board &board);
	
	inline void branch(vector<Long> &br, Long_I ind); // get index vector of the a branch ending at a node

	inline void writeSGF(const string &name);

	inline Bool next_exist(Move mov, Long_I treeInd = -1); // does next node have this mov already?

	inline Int rand_move(Long_I treeInd = -1); // make a random move for a node

	inline void winner(Long_I node); // analyse who has winning strategy for a node

	inline Int score_x2(Long_I treeInd = -1) const
	{ return get_board(treeInd < 0? m_treeInd : treeInd).score_x2(); }

	~Tree() {}
};

// create 0-th node: empty board
Tree::Tree() : m_treeInd(0)
{
	Node node; node.init();
	m_nodes.push_back(node);
	Board board(board_Nx(), board_Ny());
	m_pool.push(board, -3, 0, 0);
}

inline Long Tree::last(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	const Node &node = (treeInd < 0) ? m_nodes[m_treeInd] : m_nodes[treeInd];
	return node.last(forkInd);
}


inline void Tree::disp_board(Long_I treeInd /*optional*/)
{
	Char x, y;
	const Long treeInd1 = treeInd < 0 ? m_treeInd : treeInd;
	if (m_nodes[treeInd1].isinit())
		cout << "(empty)";
	else {
		if (who() == BLACK)
			cout << "(black ";
		else if (who() == WHITE)
			cout << "(white ";
		else
			error("Tree:disp(): illegal side name!");

		if (m_nodes[treeInd1].type() == 1)
			cout << "pass)";
		else
			cout << "[" << Int(x) << "," << Int(y) << "] )";
	}
	
	cout << "\n\n";
	get_board(treeInd).disp();
	cout << '\n' << endl;
};

// return -1 if game ends, 0 if not
inline Int Tree::pass(Long_I treeInd /*optional*/)
{
	Node node;
	const Long treeInd1 = (treeInd < 0) ? m_treeInd : treeInd;
	node.pass(inv(who()), treeInd1, m_nodes[treeInd1].poolInd());
	m_nodes.push_back(node);
	if(treeInd < 0) ++m_treeInd;
	// check double pass (game ends)
	if (m_nodes[treeInd1 - 1].ispass()) {
		m_nodes[treeInd1].push_next(-1);
		return -1;
	}
	return 0;
}

// return 0 if legal, -1 if illegal and show error
// already has bound checking
inline Int Tree::place(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = (treeInd1 < 0) ? m_treeInd : treeInd;
	Node node;
	Board board = get_board();
	// first move
	if (treeInd1 == 0) {
		board.place(x, y, BLACK);
		m_pool.push(board, -3, 0, treeInd1);
		node.place(x, y, BLACK, -1, m_pool.size()-1);
		m_nodes.push_back(node);
		if (treeInd < 0) m_treeInd = 1;
		return 0;
	}

	// update board and check illegal move (Ko no checked!)
	if (board.place(x, y, inv(who())) != 0) {
		error("Tree::place(x,y): illegal move!");
		return -1;
	}

	// check Ko
	Long orderInd;
	Long treeInd_ko;
	ret = check_ko(treeInd_ko, orderInd, treeInd1, board);
	if (ret < 0) { // board already exists
		if (ret == -1) { // not a ko
			node.push_next(treeInd_ko);
		}
		else if (ret == -2) { // is a ko
			node.push_next(-treeInd_ko - 1);
		}
		return 0;
	}
	
	// board is new
	// push board to pool
	m_pool.push(board, ret, orderInd, treeInd1+1);
	node.place(x, y, inv(who()), treeInd1, m_pool.size() + 1);

	// add Node to tree
	node.push_next(treeInd1 + 1);
	m_nodes.push_back(node);
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
		if (treeInd == treeInd2) return 1; // found treeInd2
		if (m_nodes[treeInd].last(0) == -1) return 0; // reached top of tree
		treeInd = m_nodes[treeInd].last(0); // single line, go up
	}
	for (i = 0; i < m_nodes[treeInd].nlast(); ++i) {
		if (islinked(treeInd1, last(treeInd, i)) == 1)
			return 1;
	}
}

// 'nodes[treeInd]' will produce 'board' in the next move
// if board is new, return 0
// if board already exists, output tree index of the same board
// if it is not a ko, return -1, if a ko is found, return -2
// (in case of multiple upward fork, there might be no ko for some branches!)
inline Int Tree::check_ko(Long_O &treeInd1, Long_O &orderInd, Long_I treeInd, const Board &board)
{
	Int ret = m_pool.search(treeInd1, orderInd, board);
	// board already exists
	if (ret == 0) {
		// a Ko!
		if (islinked(treeInd1, treeInd)) return -2;
		return -1;
	}
	return 0;
}

// if multiple paths exists choose the first one for now
inline void Tree::branch(vector<Long> &br, Long_I ind)
{
	Int i, Nbr;
	Long ind1 = m_treeInd;
	br.resize(0);
	for (i = 0; i < 10000; ++i) {
		br.push_back(ind1);
		ind1 = m_nodes[ind1].last(0);
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

	// write a branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Long> br;
	branch(br, m_treeInd);
	for (i = 0; i < br.size(); ++i) {
		// black moves
		fout << "  ;" << BW;
		if (m_nodes[br[i]].ispass()) // pass
			fout << "[]\n";
		else
			fout << '[' << char('a' + x) << char('a' + y) << "]\n";
		BW = BW == 'B' ? 'W' : 'B';
	}
	fout << ")\n";
	fout.close();
}

inline Bool Tree::next_exist(Move mov, Long_I treeInd /*optional*/)
{
	Int i;
	Node &node = treeInd < 0 ? m_nodes[m_treeInd] : m_nodes[treeInd];
	for (i = 0; i < node.nnext(); ++i)
		if (m_nodes[node.next(i)].move() == mov)
			return true;
	return false;
}

// return 0 if successful
// return -1 if there is all legal moves are already in the tree
Int Tree::rand_move(Long_I treeInd /*optional*/)
{
	Bool exist, exist_pass = false;
	Int i, j, Nx = board_Nx(), Ny = board_Ny(), Nxy = Nx*Ny;
	Char x0, y0, x, y;
	VecInt xy;
	Node & node = treeInd < 0 ? m_nodes[m_treeInd] : m_nodes[treeInd];

	// random sequence of all grid points on board
	randPerm(xy, Nxy);

	// search xy for a new legal move (not in Node::m_next)
	for (i = 0; i < Nxy; ++i) {		
		x = xy[i] % Nx; y = xy[i] / Nx;
		// check existence
		if (next_exist(Move(x, y))) continue;
		if (place(x, y)) continue;
		else return 0;
	}

	// no legal move exists, consider passing
	if (next_exist(Move(PASS))) return -1;
	return pass();
}

void Tree::winner(Long_I ind)
{
	// TODO: this might be difficult
}

int main()
{
	Int i;
	board_Nx(3); board_Ny(3); // set board size
	Tree tree; tree.disp_board();
	cout << "black score: " << tree.score_x2()/2. << "\n\n";
	
	for (i = 0; i < 100; ++i) {
		if (tree.rand_move() != 0) break;
		tree.disp_board();
		cout << "black score: " << tree.score_x2() / 2. << "\n\n";
	}
	
	cout << "game over!" << "\n\n";
	tree.writeSGF("test.sgf");
}
