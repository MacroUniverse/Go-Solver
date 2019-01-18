#include "move.h"

// a node in the game tree
// a fork index (forkInd) is an index for m_last or m_next
// Move is the move that caused the current board
class Node
{
private:
	Who m_who; // who's turn is this?
	vector<Long> m_next; // -1: end node
	vector<Long> m_last; // for 0-th node: undefined
	vector<Move> m_last_mov; // moves of m_last that leads to this node
	Long m_poolInd; // pool index, board stored in Pool::m_board[m_pool_ind]
public:
	// solution related
	Sol m_sol; // Sol::GOOD/Sol::BAD/Sol::FAIR
	Int m_score2; // an over-estimation of final score (two gods playing), -1 means unclear
	Sol m_best_child_sol; // solution of the best child that are already solved
	Int m_best_child_sco2; // score of the best child that are already solved
	Bool m_all_child_exist; // all legal children exist in the tree

	Node() : m_sol(Sol::UNKNOWN), m_best_child_sol(Sol::BAD), m_best_child_sco2(-1), m_all_child_exist(false), m_score2(-1) {}

	// properties
	// const Move & move() const { return *this; }
	Who who() const { return m_who; }
	Bool isinit() const { return m_last_mov[0].isinit(); }
	Bool ispass(Int_I forkInd = 0) const { return m_last_mov[forkInd].ispass(); }
	Bool isedit(Int_I forkInd = 0) const { return m_last_mov[forkInd].isedit(); }
	Bool isplace(Int_I forkInd = 0) const { return m_last_mov[forkInd].isplace(); }
	Bool x(Int_I forkInd = 0) const { return m_last_mov[forkInd].x(); }
	Bool y(Int_I forkInd = 0) const { return m_last_mov[forkInd].y(); }
	Move move(Int_I forkInd = 0) const // get the move that leads to this node
	{
		return m_last_mov[forkInd];
	}
	Int parent(Long_I treeInd) const; // search a parent by tree index
	Long poolInd() const { return m_poolInd; }
	Int nlast() const { return m_last.size(); }
	Long last(Int_I ind) const { return m_last[ind]; }
	Int nnext() const { return m_next.size(); }
	inline Long next(Int_I ind) const; // tree index for a child
	inline Bool isend() const; // is this a bottom node (end of game)?
	inline Bool is_next_ko(Int_I ind) const; // is this a ko link?

											 // set
	void init(Long_I poolInd)
	{
		m_last.resize(0); m_last.push_back(-1); m_last_mov.push_back(Move(Act::INIT));
		m_who = Who::NONE; m_poolInd = poolInd;
	}

	void push_last(const Move &mov, Long_I treeInd) { m_last.push_back(treeInd); m_last_mov.push_back(mov); }
	void push_next(Long_I treeInd) { m_next.push_back(treeInd); }

	void place(Char_I x, Char_I y, Who_I who, Long_I treeInd, Long_I poolInd) // already has bound checking
	{
		m_who = who; push_last(Move(x, y), treeInd); m_poolInd = poolInd;
	}

	void pass(Who_I who, Long_I treeInd, Long_I poolInd)
	{
		push_last(Move(Act::PASS), treeInd); m_poolInd = poolInd; m_who = who;
	}

	~Node() {}
};

// assuming the parents are in order
Int Node::parent(Long_I treeInd) const
{
	Long forkInd;
	lookupInt(forkInd, m_last, treeInd);
	return forkInd;
}

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
