#include "move.h"

// a node in the game tree
// a fork index (forkInd) is an index for m_last or m_next
// Move is the move that caused the current board
class Node
{
private:
	// === data members ===

	Who m_who; // who played this node
	vector<Long> m_next; // tree indices to next nodes (-1: end node)
	vector<Long> m_last; // for 0-th node: undefined
	vector<Move> m_last_mov; // moves of m_last that leads to this node
	Long m_poolInd; // pool index, board stored in Pool::m_board[m_pool_ind]
	Trans m_trans; // transformations needed for the config
	// solution related
	Sol m_sol; // Sol::GOOD/Sol::BAD/Sol::FAIR
	Int m_score2; // an over-estimation of final score (two gods playing), -1 means unclear

public:
	Node() : m_sol(Sol::UNKNOWN), m_score2(-1) {}

	// properties
	// const Move & move() const { return *this; }
	Who who() const { return m_who; }
	Bool isinit() const { return m_last_mov[0].isinit(); }
	Bool ispass(Int_I forkInd = 0) const { return m_last_mov[forkInd].ispass(); }
	Bool isedit(Int_I forkInd = 0) const { return m_last_mov[forkInd].isedit(); }
	Bool isplace(Int_I forkInd = 0) const { return m_last_mov[forkInd].isplace(); }
	Char x(Int_I forkInd = 0) const {
		return m_last_mov[forkInd].x();
	}
	Char y(Int_I forkInd = 0) const
	{
		return m_last_mov[forkInd].y();
	}
	const Move & move(Int_I forkInd = 0) const // get the move that leads to this node
	{
		return m_last_mov[forkInd];
	}
	Int parent(Long_I treeInd) const; // search a parent by tree index
	Long poolInd() const
	{
		return m_poolInd;
	}
	const Trans & trans() const
	{
		return m_trans;
	}
	Int nlast() const { return m_last.size(); }

	Bool is_last_ko_link(Int_I ind) const
	{
		if (m_last[ind] < 0)
			return true;
		return false;
	}

	Long last(Int_I ind) const
	{
		Long ret = m_last[ind];
		if (ret < 0)
			return -ret - 1;
		else
			return ret;
	}
	Int nnext() const { return m_next.size(); }
	inline Long next(Int_I ind) const; // tree index for a child
	inline Bool isend() const; // is this a bottom node (end of game)?
	inline Bool is_next_ko(Int_I ind) const; // is this a ko link?

	// set 0-th node (empty board)
	void init()
	{
		m_last.resize(0); m_last.push_back(-1); m_last_mov.push_back(Move(Act::INIT));
		m_who = Who::NONE; m_poolInd = 0;
	}

	Int score2() const
	{
		return m_score2;
	}

	void set_sco2(Int_I score2)
	{
		m_score2 = score2;
	}

	Sol solution() const
	{
		return m_sol;
	}

	void set_solution(Sol_I sol)
	{
		m_sol = sol;
	}

	void push_last(const Move &mov, Long_I treeInd) { m_last.push_back(treeInd); m_last_mov.push_back(mov); }
	void push_next(Long_I treeInd)
	{ m_next.push_back(treeInd); }

	void set(Who_I who, Long_I poolInd, Trans_I trans)
	{
		m_who = who; m_poolInd = poolInd; m_trans = trans;
	}

	~Node() {}
};

// assuming the parents are in order
Int Node::parent(Long_I treeInd) const
{
	Int i;
	for (i = nlast() - 1; i >= 0; ++i) {
		if (last(i) == treeInd)
			return i;
	}
	error("parent not found!");
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
		return -treeInd - 1; // ko link
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
