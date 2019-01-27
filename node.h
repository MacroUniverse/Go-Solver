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

	Who who() const;

	Bool isinit() const;

	Bool ispass(Int_I forkInd = 0) const;

	Bool isedit(Int_I forkInd = 0) const;

	Bool isplace(Int_I forkInd = 0) const;

	Char x(Int_I forkInd = 0) const;
	
	Char y(Int_I forkInd = 0) const;

	// get the move that leads to this node
	const Move & move(Int_I forkInd = 0) const;

	// search a parent by tree index
	// there might be multiple links to a single parent!
	void parent(vector<Int> &forkInd, Long_I treeInd) const;

	Long poolInd() const;

	const Trans & trans() const;

	Int nlast() const { return m_last.size(); }

	Long last(Int_I forkInd) const;

	Bool is_last_ko_link(Int_I forkInd) const;

	Int nnext() const;

	// tree index for a child
	// ind = -1 : last element, ind = -2 : second last element, etc.
	Long next(Int_I ind) const;

	Bool isend() const; // is this a bottom node (end of game)?

	Bool is_next_ko_link(Int_I ind) const; // is this a ko link?

	// set 0-th node (empty board)
	void init();

	Int score2() const;

	void set_sco2(Int_I score2);

	Sol solution() const;

	void set_solution(Sol_I sol);

	void push_last(const Move &mov, Long_I treeInd);

	void push_next(Long_I treeInd);

	void set(Who_I who, Long_I poolInd, Trans_I trans);

	// change a ko link in m_next to a normal link
	void next_ko_link_2_link(Int_I forkInd);

	// change a ko link in m_last to a normal link
	void last_ko_link_2_link(Int_I forkInd);

	// change a normal link in m_last to a ko link 
	void last_link_2_ko_link(Int_I forkInd);

	// change a normal link in m_next to a ko link 
	void next_link_2_ko_link(Int_I forkInd);

	// remove one element from m_last
	void delete_last(Int_I forkInd);

	// remove one element from m_next
	void delete_next(Int_I forkInd);

	~Node() {}
};

inline Who Node::who() const
{
	return m_who;
}

inline Bool Node::isinit() const
{
	return m_last_mov[0].isinit();
}

inline Bool Node::ispass(Int_I forkInd) const
{
	return m_last_mov[forkInd].ispass();
}

inline Bool Node::isedit(Int_I forkInd) const
{
	return m_last_mov[forkInd].isedit();
}

inline Bool Node::isplace(Int_I forkInd) const
{
	return m_last_mov[forkInd].isplace();
}

inline Char Node::x(Int_I forkInd) const
{
	return m_last_mov[forkInd].x();
}

inline Char Node::y(Int_I forkInd) const
{
	return m_last_mov[forkInd].y();
}

inline const Move & Node::move(Int_I forkInd) const
{
	return m_last_mov[forkInd];
}

inline void Node::parent(vector<Int> &forkInd, Long_I treeInd) const
{
	Int i, Nlast = nlast();
	forkInd.resize(0);
	for (i = 0; i < Nlast; ++i) {
		if (last(i) == treeInd)
			forkInd.push_back(i);
	}
	if (forkInd.size() == 0)
		error("parent not found!");
}

inline Long Node::poolInd() const
{
	return m_poolInd;
}

inline const Trans & Node::trans() const
{
	return m_trans;
}

inline Long Node::last(Int_I forkInd) const
{
	Long ret = m_last[forkInd];
	if (ret < 0)
		return -ret - 1;
	else
		return ret;
}

inline Bool Node::is_last_ko_link(Int_I forkInd) const
{
	return m_last[forkInd] < 0;
}

inline Int Node::nnext() const
{
	return m_next.size();
}

inline Long Node::next(Int_I ind) const
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
inline Bool Node::is_next_ko_link(Int_I ind) const
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

inline void Node::init()
{
	m_last.resize(0); m_last.push_back(-1); m_last_mov.push_back(Move(Act::INIT));
	m_who = Who::NONE; m_poolInd = 0;
}

inline Int Node::score2() const
{
	return m_score2;
}

inline void Node::set_sco2(Int_I score2)
{
#ifdef GOS_CHECK_BOUND
	if (score2 < 0 || score2 > board_Nx()*board_Ny() * 2)
		error("illegal score2!");
#endif
	m_score2 = score2;
}

inline Sol Node::solution() const
{
	return m_sol;
}

inline void Node::set_solution(Sol_I sol)
{
	m_sol = sol;
}

inline void Node::push_last(const Move &mov, Long_I treeInd)
{
	m_last.push_back(treeInd); m_last_mov.push_back(mov);
}

inline void Node::push_next(Long_I treeInd)
{
	m_next.push_back(treeInd);
}

inline void Node::set(Who_I who, Long_I poolInd, Trans_I trans)
{
	m_who = who; m_poolInd = poolInd; m_trans = trans;
}

inline void Node::next_ko_link_2_link(Int_I forkInd)
{
#ifdef GOS_CHECK_BOUND
	if (!is_next_ko_link(forkInd))
		error("not a ko link!");
#endif
	m_next[forkInd] = next(forkInd);
}

inline void Node::last_ko_link_2_link(Int_I forkInd)
{
#ifdef GOS_CHECK_BOUND
	if (!is_last_ko_link(forkInd))
		error("not a ko link!");
#endif
	m_last[forkInd] = last(forkInd);
}

inline void Node::last_link_2_ko_link(Int_I forkInd)
{
#ifdef GOS_CHECK_BOUND
	if (is_last_ko_link(forkInd))
		error("not a normal link!");
#endif
	m_last[forkInd] = -last(forkInd) - 1;
}

inline void Node::next_link_2_ko_link(Int_I forkInd)
{
#ifdef GOS_CHECK_BOUND
	if (is_next_ko_link(forkInd))
		error("not a normal link!");
#endif
	m_next[forkInd] = -next(forkInd) - 1;
}

inline void Node::delete_last(Int_I forkInd)
{
	m_last.erase(m_last.begin() + forkInd);
	m_last_mov.erase(m_last_mov.begin() + forkInd);
}

inline void Node::delete_next(Int_I forkInd)
{
	m_next.erase(m_next.begin() + forkInd);
}
