#include "move.h"
#include "link.h"

// a node in the game tree
// a fork index (forkInd) is an index for m_last or m_next
// Move is the move that caused the current board
class Node
{
private:
	// === data members ===

	Who m_who; // who played this node
	vector<Linkp> m_next; // tree indices to next nodes (-1: end node)
	vector<Linkp> m_last; // for 0-th node: undefined
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

	Long poolInd() const;

	Bool is_ko_node() const
	{
		return is_ko_sol(solution());
	}

	const Trans & trans() const;

	Int nlast() const { return m_last.size(); }

	// link for a parent
	// forkInd = -1 : last element, forkInd = -2 : second last element, etc.
	Linkp last(Int_I forkInd = 0) const;

	Int nnext() const;

	// find forkInd of m_last for a link
	const Int last_forkInd(Linkp_I plink) const;

	// find forkInd of m_next for a link
	const Int next_forkInd(Linkp_I plink) const;

	// link for a child
	// forkInd = -1 : last element, forkInd = -2 : second last element, etc.
	Linkp next(Int_I forkInd = 0) const;

	// set 0-th node (empty board)
	void init();

	Int score2() const;

	void set_sco2(Int_I score2);

	Sol solution() const;

	void set_solution(Sol_I sol);

	void push_last(Linkp_I plink);

	void push_next(Linkp_I plink);

	void set(Who_I who, Long_I poolInd, Trans_I trans);

	// change a next link
	void set_next(Int_I forkInd, Linkp_I plink);

	// change a last link
	void set_last(Int_I forkInd, Linkp_I plink);

	// remove one link from m_last
	// will not deallocate link!
	void delete_last(Int_I forkInd);

	// remove one link from m_next
	// will not deallocate link!
	void delete_next(Int_I forkInd);

	~Node() {}
};

inline Who Node::who() const
{
	return m_who;
}

inline Bool Node::isinit() const
{
	if (m_last.size() == 1 & m_last[0]->isinit())
		return true;
	return false;
}

inline Long Node::poolInd() const
{
	return m_poolInd;
}

inline const Trans & Node::trans() const
{
	return m_trans;
}

inline Linkp Node::last(Int_I ind) const
{
	Long treeInd;
	if (ind < 0)
		return m_last[nnext() + ind];
	else
		return m_last[ind];
}

inline Int Node::nnext() const
{
	return m_next.size();
}

inline const Int Node::last_forkInd(Linkp_I plink) const
{
	for (Int i = 0; i < nlast(); ++i) {
		if (last(i) == plink) {
			return i;
		}
	}
	error("parent not found");
	return 1000000;
}

inline const Int Node::next_forkInd(Linkp_I plink) const
{
	for (Int i = 0; i < nnext(); ++i) {
		if (next(i) == plink) {
			return i;
		}
	}
	error("parent not found");
	return 1000000;
}

inline Linkp Node::next(Int_I ind) const
{
	Long treeInd;
	if (ind < 0)
		return m_next[nnext() + ind];
	else
		return m_next[ind];
}

inline void Node::init()
{
	m_last.resize(0);
	Linkp plink = Linkp::newlink();
	plink->init();
	m_last.push_back(plink);
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

inline void Node::push_last(Linkp_I plink)
{
	m_last.push_back(plink);
}

inline void Node::push_next(Linkp_I plink)
{
	m_next.push_back(plink);
}

inline void Node::set(Who_I who, Long_I poolInd, Trans_I trans)
{
	m_who = who; m_poolInd = poolInd; m_trans = trans;
}

inline void Node::set_last(Int_I forkInd, Linkp_I plink)
{
	m_last[forkInd] = plink;
}

inline void Node::set_next(Int_I forkInd, Linkp_I plink)
{
	m_next[forkInd] = plink;
}

inline void Node::delete_last(Int_I forkInd)
{
	if (forkInd < 0) {
		m_last.erase(m_last.end() + forkInd);
	}
	else {
		m_last.erase(m_last.begin() + forkInd);
	}
}

inline void Node::delete_next(Int_I forkInd)
{
	if (forkInd < 0) {
		m_next.erase(m_next.end() + forkInd);
	}
	else {
		m_next.erase(m_next.begin() + forkInd);
	}
}
