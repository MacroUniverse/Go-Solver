#pragma once
#include "node.h"
#include "pool.h"
#include "boardref.h"
#include "SLISC/search.h"

// game tree
// a tree index is an index for m_nodes (treeInd), this index should never change for the same node
// if any node has nlast() > 1, it creates an "upward fork"
// TODO: make sure no two nodes have the same situation
// TODO: treeInd = 0 should be the empty board!
class Tree
{
private:
	// === data members ===
	Long m_treeInd; // index of the current step (already played) in m_data (init: -1)
	vector<Node> m_nodes;
	Pool m_pool;

public:
	Tree();

	// global work space, TODO: should not be members
	// keep track of all unresolved ko links
	vector<Long> m_ko_link_to;
	vector<Long> m_ko_link_from;

	// ko nodes with all ko links resolved
	vector<Long> m_clean_ko_node;
	vector<Int> m_clean_ko_node_sco2;
	vector<Sol> m_clean_ko_node_sol;

	// === const functions ===

	// return a reference to the transformed Board of a node
	// rotation and flip are transformations needed to transform the board back
	BoardRef get_board(Long_I treeInd = -1) const;

	Long index() const { return m_treeInd; } // return current node index

	Long nnode() const { return m_nodes.size(); } // return number of nodes in the tree

	Long max_treeInd() const { return m_nodes.size() - 1; };

	inline Bool is_ko_node(Long_I treeInd = -1) const
	{
		return is_ko_sol(solution(def(treeInd)));
	}

	inline Bool is_ko_child(Long_I treeInd = -1, Int_I forkInd = 0) const
	{
		Long treeInd1 = def(treeInd);
		const Node & node = Tree::node(treeInd1);
		return !node.is_next_ko_link(forkInd) && is_ko_node(node.next(forkInd));
	}

	inline Bool isend(Long_I treeInd = -1) const; // if a node is an end node

	inline Bool is_next_ko_link(Long_I treeInd = -1, Int_I forkInd = 0) const;

	inline Node & node(Long_I treeInd = -1) // return a reference of a node
	{
		return m_nodes[def(treeInd)];
	}

	inline const Node & node(Long_I treeInd = -1) const // return a reference of a node
	{
		return m_nodes[def(treeInd)];
	}

	inline Int nlast(Long_I treeInd = -1) const
	{
		return m_nodes[def(treeInd)].nlast();
	}

	inline Long last(Long_I treeInd = -1, Int_I forkInd = 0) const; // return an index of the last node of a node

	const Node & lastNode(Long_I treeInd = -1, Int_I forkInd = 0) // return a reference of the last node of a node
	{
		return m_nodes[last(def(treeInd), forkInd)];
	}

	// return an index of the next node of a node, fork can be negative
	inline Long next(Long_I treeInd = -1, Int_I forkInd = 0) const;

	const Node & nextNode(Long_I treeInd = -1, Int_I forkInd = 0) const // return a reference of the next node of a node
	{
		return m_nodes[next(def(treeInd), forkInd)];
	} // forkInd can be negative

	// find the parent fork index of a child
	Int inv_link(Long_I treeInd_from, Int_I forkInd) const
	{
		Int i, j, forkInd_inv = 0;
		const Long next_treeInd = next(treeInd_from, forkInd);
		const Node & next_node = node(next_treeInd);
		vector<Int> forkInds;
		next_node.parent(forkInds, treeInd_from);
		if (forkInds.size() == 1) {
			forkInd_inv = forkInds[0];
			return forkInd_inv;
		}
		else {
			// multiple links between two nodes
			// count number of repeated links
			// doesn't matter which is which, as long as they are one to one linked
			for (j = 0; j < forkInd; ++j) {
				if (next(treeInd_from, j) == next_treeInd)
					++forkInd_inv;
			}
			return forkInds[forkInd_inv];
		}
	}

	// return next move of a node
	Move nextMove(Long_I treeInd = -1, Int_I forkInd = 0) const
	{
		const Long treeInd1 = def(treeInd);
		Int inv_fork = inv_link(treeInd1, forkInd);
		return nextNode(treeInd1, forkInd).move(inv_fork);
	}

	Who who(Long_I treeInd = -1) const // who played the node
	{
		return m_nodes[def(treeInd)].who();
	}

	// who relative to config, not situation (difference when color flip exists)
	Who who_config(Long_I treeInd = -1) const
	{
		const Long treeInd1 = def(treeInd);
		if (treeInd1 == 0)
			error("unhandled case!");
		if (get_board(treeInd1).trans().flip())
			return ::next(who(treeInd1));
		return who(treeInd1);
	}

	inline void disp_board(Long_I treeInd = -1) const; // display board

	// return 0 if new node created
	// return -1 if double pass caused game end
	// return -2 if double pass attempted but game did not finish, this is treated as a ko
	// return 1 if linked to an existing normal node or clean ko node.
	// return 2 if ko link is created
	// return 3 if linked to an (unclean) ko node
	inline Int pass(Long_I treeInd = -1);

	inline Int check(Char_I x, Char_I y, Long_I treeInd = -1);

	// return 0 if new node created (m_treeInd jumped)
	// return 1 if linked to an existing normal node or clean ko node (m_treeInd jumped)
	// return 2 if linked to a (unclean) ko node
	// return -1 if illegal (nothing changes)
	// return -2 if a ko is found and linked (m_treeInd unchanged)
	// already has bound checking
	inline Int place(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int islinked(Long_I treeInd1, Long_I treeInd2); // check if node treeInd1 can lead to node treeInd2

														   // check if same board already exists in the Pool and decide if it is a Ko
	inline Int check_ko(Int_O &search_ret, Long_O &treeInd_match, Long_O &orderInd, Long_I treeInd, Config_I &config, Bool_I flip);

	Node & nextNode(Long_I treeInd = -1, Int_I forkInd = 0) // return a reference of the next node of a node
	{
		return m_nodes[next(def(treeInd), forkInd)];
	}

	// create normal link between two nodes
	void link(Long_I treeInd_from, Move_I mov, Long_I treeInd_to)
	{
		node(treeInd_from).push_next(treeInd_to);
		node(treeInd_to).push_last(mov, treeInd_from);
	}

	// unlink two nodes
	Move unlink(Long_I treeInd, Int_I forkInd)
	{
		Long treeInd1 = def(treeInd);
		vector<Int> up_forks;
		Move mov;
		Node & node = Tree::node(treeInd1);
		Int forkInd1 = forkInd < 0 ? node.nnext() + forkInd : forkInd;
		Node & next_node = nextNode(treeInd1, forkInd1);
		Int inv_forkInd = inv_link(treeInd1, forkInd1);
		node.delete_next(forkInd1);
		mov = next_node.move(inv_forkInd);
		next_node.delete_last(inv_forkInd);
		return mov;
	}

	void ko_link(Long_I treeInd_from, Move_I mov, Long_I treeInd_to)
	{
		node(treeInd_from).push_next(-treeInd_to-1);
		node(treeInd_to).push_last(mov, -treeInd_from-1);
	}

	inline void branch(vector<Long> &br, Long_I treeInd = -1); // get an index vector of the a branch ending at a node

	inline void writeSGF_old(const string &name); // write a branch to file

	inline void writeSGF(const string &name); // write the whole tree to file

	inline void writeSGF0(VecBool &check, ofstream &fout, Long_I treeInd = -1); // internal function called by writeSGF();

	inline void writeSGF01(ofstream &fout, Long_I treeInd, const string &prefix = ""); // internal function called by writeSGF0();

	inline void writeSGF0_link(ofstream &fout, Long_I treeInd_from, Int_I forkInd, const string &prefix);

	inline Bool next_exist(Move mov, Long_I treeInd = -1); // does next node have this mov already?

	// make a random move for a node
	// only pass if all other moves are onsidered
	inline Int rand_move(Long_I treeInd = -1);

	// smarter random move for a node
	// will not do a dumb move (dumb eye filling or dumb big eye filling)
	// return 0 if successful and new node created
	// return 1 if linked to existing normal node or clean ko node
	// return 2 if a ko link is created
	// return 3 if sucessful but is a dumb move
	// return 4 if linked to an existing non-clean ko node
	// return -1 if all legal moves already exists
	// return -2 if passed and game ends
	// return -3 if double pass not allowed and ko link created
	inline Int rand_smart_move(Long_I treeInd = -1);

	// randomly play to the end of the game from a node
	// m_treeInd will be changed to the end node!
	// the end node will have the correct Node::m_win
	// using rand_move1() currently
	// set "out = true" for output to terminal and test.sgf
	inline Int rand_game(Long_I treeInd = -1, Bool_I out = false);

	inline void solve_end(Long_I treeInd = -1); // solve a bottom node

	// analyse who has winning strategy for a node
	// might m_treeInd be changed? it shouldn't
	// this is a recursive function
	// using rand_smart_move() to generate moves until a better one is available
	// assuming there is no branch after treeInd for now
	// return 0 if successful
	// return 1 if double pass (two nodes linked by the first pass both solved)
	// return -1 if is a ko node with unresolved ko links
	// return 2 if is a ko node with all ko links resolved
	// return 3 if is a forbidden node
	Int solve(Long_I treeInd = -1);

	// solve an existing ko node or make it a clean ko node
	// try to solve a ko node (treeInd_to) when it has been linked
	// the new link is from treeInd_from
	// return 0 if solved to solvable node
	// return 1 if solved to a clean ko node and pushed
	// return 2 if this is already a clean ko node
	Int solve_ko(Long_I treeInd_from, Int_I forkInd);

	// internal recursive function called by solve_ko()
	// solve a ko node
	// this function is similar to solve()
	// return 0 if solved
	// return 1 if is still a ko node
	Int solve_ko0(Long_I treeInd1);

	// change a ko link to normal link
	inline void ko_link_2_link(Long_I treeInd_from, Long_I treeInd_to)
	{
		Int i;
		Node & node_from = node(treeInd_from);
		Node & node_to = node(treeInd_to);
		Bool check = false;
		for (i = node_from.nnext() - 1; i >= 0; --i) {
			if (node_from.next(i) == treeInd_to) {
				node_from.next_ko_link_2_link(i);
				check = true;
				break;
			}
		}
		if (!check)
			error("ko link head not found!");
		check = false;
		for (i = node_to.nlast() - 1; i >= 0; --i) {
			if (node_to.last(i) == treeInd_from) {
				node_to.last_ko_link_2_link(i);
				check = true;
				break;
			}
		}
		if (!check)
			error("ko link end not found!");
	}

	// change a normal link to ko link
	inline void link_2_ko_link(Long_I treeInd_from, Long_I treeInd_to)
	{
		Int i;
		Node & node_from = node(treeInd_from);
		Node & node_to = node(treeInd_to);
		Bool check = false;
		for (i = node_from.nnext() - 1; i >= 0; --i) {
			if (node_from.next(i) == treeInd_to) {
				node_from.next_link_2_ko_link(i);
				check = true;
				break;
			}
		}
		if (!check)
			error("ko link head not found!");
		check = false;
		for (i = node_to.nlast() - 1; i >= 0; --i) {
			if (node_to.last(i) == treeInd_from) {
				node_to.last_link_2_ko_link(i);
				check = true;
				break;
			}
		}
		if (!check)
			error("ko link end not found!");
	}

	// push a clean ko node to record
	void push_clean_ko_node(Long_I treeInd1)
	{
		m_clean_ko_node.push_back(treeInd1);
		m_clean_ko_node_sco2.push_back(score2(treeInd1));
		m_clean_ko_node_sol.push_back(solution(treeInd1));
	}

	void delete_clean_ko_node(Int_I cleanInd)
	{
		m_clean_ko_node.erase(m_clean_ko_node.begin() + cleanInd);
		m_clean_ko_node_sco2.erase(m_clean_ko_node_sco2.begin() + cleanInd);
		m_clean_ko_node_sol.erase(m_clean_ko_node_sol.begin() + cleanInd);
	}

	// if the node is a clean ko node
	// return -1 if not clean
	// return index to m_clean_ko_node[] if clean
	Int check_clean_ko_node(Long_I treeInd = -1)
	{
		Int i, N = m_clean_ko_node.size();
		Long treeInd1 = def(treeInd);
		for (i = 0; i < N; ++i) {
			if (m_clean_ko_node[i] == treeInd1)
				return i;
		}
		return -1;
	}

	// try to remove clean ko node record
	// to nothing if not a clean ko node
	// return 0 if removed
	// return 1 if nothing is done
	Int rm_clean_ko_node(Long_I treeInd = -1)
	{
		Int ind = check_clean_ko_node(def(treeInd));
		if (ind >= 0) {
			m_clean_ko_node.erase(m_clean_ko_node.begin() + ind);
			return 0;
		}
		return 1;
	}

	// get the score of a clean ko node
	// don't trust the score2 function
	// 'which' is the return from check_clean_ko_node()
	Int clean_ko_node_sco2(Int_I cleanInd) const
	{
		return m_clean_ko_node_sco2[cleanInd];
	}

	Sol clean_ko_node_sol(Int_I cleanInd) const
	{
		return m_clean_ko_node_sol[cleanInd];
	}

	// the winner of a node
	Who winner(Long treeInd = -1) const;

	// calculate and update score for a node
	void calc_score(Long_I treeInd = -1);

	// update solution for a node based on scores and komi
	void calc_sol(Long_I treeInd = -1);

	// update solution for a ko node based on scores and komi
	void calc_ko_sol(Long_I treeInd = -1);

	// stored doubled score of a node (might not be up to date)
	Int score2(Long_I treeInd = -1) const;

	// set score of a node
	void set_score2(Int score2, Long_I treeInd = -1);

	// stored solution of a node (might not be up to date)
	Sol solution(Long_I treeInd = -1) const;

	// set solution of a node
	void set_solution(Sol_I sol, Long_I treeInd = -1);

	// if a node is solved (GOOD/BAD/FAIR)
	Bool solved(Long_I treeInd = -1) const;

	Sol & best_child_sol(Long_I treeInd = -1)
	{
		Int i, sco2;
		Sol best_sol = Sol::BAD, sol;
		const Node & node = Tree::node(def(treeInd));
		for (i = node.nnext(); i >= 0; --i)
		{
			sol = solution(node.next(i));
			if (best_sol - sol < 0) {
				best_sol = sol;
			}
		}
		return best_sol;
	}

	Int & best_child_sco2(Long_I treeInd = -1)
	{
		Int i, sco2, max_sco2 = -10000;
		const Node & node = Tree::node(def(treeInd));
		for (i = node.nnext(); i >= 0; --i)
		{
			sco2 = score2(node.next(i));
			if (max_sco2 < sco2) {
				max_sco2 = sco2;
			}
		}
		return max_sco2;
	}

	// set default argument treeInd
	Long def(Long_I treeInd) const
	{
		return treeInd < 0 ? m_treeInd : treeInd;
	}
	
	// see if a node resolves a ko link, and resolve it
	Int resolve_ko(Long_I treeInd = -1)
	{
		Int i, Nko = m_ko_link_to.size();
		Long_I treeInd1 = def(treeInd);
		for (i = 0; i < Nko; ++i) {
			if (m_ko_link_to[i] == treeInd1) {
				m_ko_link_from.erase(m_ko_link_from.begin() + i);
				m_ko_link_to.erase(m_ko_link_to.begin() + i);
				--Nko;
				break;
			}
		}
		return Nko;
	}

	// shift downstream ko links to a node (treeInd_to)
	// change all downstream ko links to the upstream to normal links
	// and change normal links from all parents to ko links
	// this node must not be a clean ko node
	void shift_ko_links(Long_I treeInd_from, Int_I forkInd)
	{
		Int i;
		Long treeInd_to = next(treeInd_from, forkInd);
		Node & node = Tree::node(treeInd_to);
		Move mov = unlink(treeInd_from, forkInd);
		// destroy ko links from downstream to upstream
		shift_ko_links0(treeInd_to, treeInd_to);
		// change all parents as descendents with a ko link
		for (i = node.nlast() - 1; i >= 0; --i) {
			if (!node.is_last_ko_link(i)) {
				link_2_ko_link(node.last(i), treeInd_to);
			}
		}
		link(treeInd_from, mov, treeInd_to);
	}

	// internal recursive function called by shift_ko_links()
	// treeInd1 is the node in shift_ko_links
	void shift_ko_links0(Long_I &treeInd1, Long_I treeInd)
	{
		Int i;
		Long ko_treeInd;
		Node & node = Tree::node(treeInd);
		for (i = 0; i < node.nnext(); ++i) {
			if (is_ko_child(treeInd, i)) {
				// found a ko child
				shift_ko_links0(treeInd1, node.next(i));
			}
			else if (node.is_next_ko_link(i)) {
				// found a ko link
				ko_treeInd = node.next(i);
				if (ko_treeInd == treeInd) {
					continue;
				}
				else if (islinked(ko_treeInd, treeInd)) {
					// change ko link to normal link
					ko_link_2_link(treeInd, ko_treeInd);
				}
			}
			// ignoring normal child
		}
	}

	~Tree() {}
};

BoardRef Tree::get_board(Long_I treeInd) const
{
	Long treeInd1 = def(treeInd);
	const Config & config = m_pool(m_nodes[treeInd1].poolInd());
	const Trans & trans = m_nodes[treeInd1].trans();
	BoardRef board_ref(config, trans);
	return board_ref;
}

// create 0-th node: empty board
Tree::Tree() : m_treeInd(0)
{
	Board board; board.init();
	m_pool.push(board, -3, 0, Who::NONE, 0);
	m_nodes.push_back(Node()); m_nodes.back().init();
}

inline Bool Tree::isend(Long_I treeInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].isend())
		return true;
	return false;
}

inline Bool Tree::is_next_ko_link(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].is_next_ko_link(forkInd))
		return true;
	return false;
}

inline Long Tree::last(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	const Node &node = m_nodes[def(treeInd)];
	return node.last(forkInd);
}

inline Long Tree::next(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	return node(def(treeInd)).next(forkInd);
}

inline void Tree::disp_board(Long_I treeInd /*optional*/) const
{
	Char x, y;
	Int i;
	Long treeInd1 = def(treeInd);
	const Node & node = m_nodes[treeInd1];
	for (i = 0; i < nlast(treeInd1); ++i) {
		cout << i << ": ";
		if (node.isinit())
			cout << "(empty)";
		else {
			if (who(treeInd1) == Who::BLACK)
				cout << "(black ";
			else if (who(treeInd1) == Who::WHITE)
				cout << "(white ";
			else
				error("Tree:disp(): illegal side name!");

			if (node.ispass(i))
				cout << "pass)" << endl;
			else if (node.isedit(i))
				cout << "edit board)" << endl;
			else {
				cout << "[" << Int(node.x(i)) << "," << Int(node.y(i)) << "])" << endl;
			}
		}
	}

	get_board(treeInd1).disp();
	cout << '\n' << endl;
};

inline Int Tree::pass(Long_I treeInd /*optional*/)
{
	Int i, ret;
	Long ko_treeInd;
	const Long treeInd1 = def(treeInd);
	const BoardRef board = get_board(treeInd1);
	Node & node = Tree::node(treeInd1);

	// check double pass
	for (i = 0; i < node.nlast(); ++i) {
		if (node.move(i).ispass()) {
			// double passed!
			if (!board.is_game_end()) {
				// game not ended, build a ko link
				ko_treeInd = last(treeInd1, i);
				ko_link(treeInd1, Move(Act::PASS), ko_treeInd);
				return -2;
			}
			// two situations have the same scores and solutions!
			calc_score(treeInd1);
			calc_sol(treeInd1);
			set_score2(inv_score2(node.score2()), last(treeInd1, i));
			set_solution(inv_sol(node.solution()), last(treeInd1, i));
			return -1;
		}
	}

	// check ko
	Int search_ret;
	Long orderInd, treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd1, board.config(), board.trans().flip());

	if (ret == -1) { // situation exists, not a ko
		link(treeInd1, Move(Act::PASS), treeInd_found);
		if (is_ko_node(treeInd_found) && check_clean_ko_node(treeInd_found) == -1) {
			// linked to a ko node but not clean
			return 2;
		}
		else {
			// linked to a normal node or a clean ko node
			return 1;
		}
	}
	else if (ret == -2) { // situation exists, is a ko
		ko_link(treeInd1, Move(Act::PASS), treeInd_found);
		return 2;
	}
	else if (ret == 0) { // configuration does not exist
		error("impossible case, configuration must exist!");
	}
	else if (ret != -3)
		error("unknown return!");

	// normal first pass (ret == -3): configuration exists, new situation

	m_nodes.push_back(Node());
	m_nodes.back().set(::next(who(treeInd1)), m_pool.poolInd(orderInd), board.trans());
	link(treeInd1, Move(Act::PASS), max_treeInd());
	m_pool.link(orderInd, ::next(who_config(treeInd1)), max_treeInd());
	return 0;
}

// check if a placing is legal, or how many stones will be dead
// same check already exists for place()
// Ko is not considered!
// if legal, return the number of stones that can be removed ( >= 0)
// return -1 if occupied, do nothing
// return -2 if no qi, do nothing
inline Int Tree::check(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = def(treeInd);
	// first move
	if (treeInd1 == 0)
		return 0;
	// update board and check illegal move (Ko no checked!)
	return get_board(treeInd1).check(x, y, ::next(who(treeInd1)));
}

inline Int Tree::place(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = def(treeInd);
	Board board;
	board = get_board(treeInd1);

	Node & node = Tree::node(treeInd1);
	Who who = Tree::who(treeInd1), next_who = ::next(who);

	// first move
	if (treeInd1 == 0) {
		board.place(x, y, Who::BLACK);
		m_pool.push(board, 1, -1, Who::BLACK, 1);
		m_nodes.push_back(Node());
		m_nodes.back().set(Who::BLACK, m_pool.size() - 1, board.trans());
		link(treeInd1, Move(x, y), max_treeInd());
		return 0;
	}

	// update board and check illegal move (Ko no checked!)
	if (board.place(x, y, next_who))
		return -1;

	// check Ko
	Int search_ret;
	Long orderInd;
	Long treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd1, board.config(), board.trans().flip());
	if (ret < 0) { // board already exists
		if (ret == -1) { // not a ko
			link(treeInd1, Move(x, y), treeInd_found);
			if (is_ko_node(treeInd_found) && check_clean_ko_node(treeInd_found) == -1) {
				// linked to a ko node but not clean
				return 2;
			}
			else {
				// linked to a normal node or a clean ko node
				return 1;
			}
		}
		else if (ret == -2) { // is a ko
			ko_link(treeInd1, Move(x, y), treeInd_found);
			return -2;
		}
		// ret == -3: not the same player, continue
	}

	// new situation
	m_nodes.push_back(Node()); // add Node to tree

	if (ret == -3) { // configuration exists
		Who next_who_config;
		if (board.trans().flip())
			next_who_config = ::next(next_who);
		else
			next_who_config = next_who;
		m_pool.link(orderInd, next_who_config, max_treeInd());
		m_nodes.back().set(next_who, m_pool.poolInd(orderInd), board.trans());
	}
	else { // configuration does not exist
		m_pool.push(board, search_ret, orderInd, next_who, max_treeInd());
		m_nodes.back().set(next_who, m_pool.size() - 1, board.trans());
	}

	link(treeInd1, Move(x, y), max_treeInd());
	return 0;
}

// return 0 if not linked
// does not check ko linked parent
// return 1 if node treeInd1 can lead to node treeInd2
// upward search from node treeInd2 is most efficient
// this is a recursive function to deal with upward fork
inline Int Tree::islinked(Long_I treeInd1, Long_I treeInd2)
{
	Long i, treeInd = treeInd2;
	for (i = 0; i < 10000; ++i) {
		if (treeInd == treeInd1) return 1; // found treeInd2
		if (m_nodes[treeInd].nlast() > 1) break; // multiple upward fork
		if (treeInd == 0) return 0; // reached top of tree
		if (m_nodes[treeInd].is_last_ko_link(0))
			error("lonely parent!");
		treeInd = m_nodes[treeInd].last(0); // single line, go up
	}
	for (i = 0; i < m_nodes[treeInd].nlast(); ++i) {
		if (m_nodes[treeInd].is_last_ko_link(i))
			continue;
		if (islinked(treeInd1, last(treeInd, i)) == 1)
			return 1;
	}
	return false;
}

// 'nodes[treeInd]' will produce 'config' in the next move
// if board already exists, output tree index of the same board
// return 0 if this is a new configuration
// return -1 if situation exists but not a ko
// return -2 if situation exists and is a ko
// return -3 if this is a new situation of an existing configuration
inline Int Tree::check_ko(Int_O &search_ret, Long_O &treeInd_match, Long_O &orderInd,
	Long_I treeInd, Config_I &config, Bool_I flip)
{
	Long poolInd;
	search_ret = m_pool.search(poolInd, orderInd, config);
	if (search_ret == 0) {
		// config already exists
		if (!flip)
			treeInd_match = m_pool.treeInd(poolInd, ::next(who(treeInd)));
		else
			treeInd_match = m_pool.treeInd(poolInd, who(treeInd));
			
		if (treeInd_match < 0)
			return -3;
		if (islinked(treeInd_match, treeInd))
			return -2; // Ko!
		return -1; // no Ko
	}
	poolInd = -1;
	return 0;
}

// if multiple paths exists choose the first one for now
inline void Tree::branch(vector<Long> &br, Long_I treeInd /*optional*/)
{
	Int i, Nbr;
	Long treeInd1 = def(treeInd);
	br.resize(0);
	for (i = 0; i < 10000; ++i) {
		if (treeInd1 == 0) break;
		br.push_back(treeInd1);
		if (m_nodes[treeInd1].nlast() == 0) error("unknown error!");
		treeInd1 = m_nodes[treeInd1].last(0);
	}
	Nbr = br.size();
	for (i = Nbr / 2 - 1; i > -1; --i) {
		SWAP(br[Nbr - i - 1], br[i]);
	}
}

inline void Tree::writeSGF_old(const string &name)
{
	Char Nx = board_Nx(), Ny = board_Ny();
	ofstream fout(name);
	fout << "(\n";
	fout << ";GM[1]FF[4]CA[UTF-8]AP[]KM[0]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Int(Nx) << ":" << Int(Ny) << "]";
	fout << "DT[]\n";

	// write a branch
	Char BW = 'B'; // letter B or letter W
	Int i; Char x, y;
	vector<Long> br;
	branch(br);
	for (i = 0; i < br.size(); ++i) {
		// black moves
		fout << "  ;" << BW;
		Node & node = m_nodes[br[i]];
		if (node.ispass()) // pass
			fout << "[]\n";
		else
			fout << '[' << char('a' + node.x()) << char('a' + node.y()) << "]\n";
		BW = BW == 'B' ? 'W' : 'B';
	}
	fout << ")\n";
	fout.close();
}

inline void Tree::writeSGF(const string &name)
{
	VecBool check(nnode(), false);
	Char Nx = board_Nx(), Ny = board_Ny();
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[" << 0.5*komi2() << "]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Int(Nx) << ":" << Int(Ny) << "]";
	fout << "DT[]\n";

	Char BW; // letter B or letter W
	Int i; Char x, y;

	fout.flush(); // debug
	writeSGF0(check, fout, 1);
	if (count(check) != nnode() - 1)
		error("writeSGF() nodes number does not match!");

	fout << ")\n";
	fout.close();
}

// return max node index written
// nodes should be written in the order of treeIndex
// recursively write the tree
inline void Tree::writeSGF0(VecBool &check, ofstream &fout, Long_I treeInd)
{
	using slisc::lookupInt;
	Int i, nnext;
	Long treeInd1 = def(treeInd), treeInd2, dummy;

	// write one node
	writeSGF01(fout, treeInd1);
	check[treeInd1] = true;

	// go down the branch if no fork
	for (i = 0; i < 10000; ++i) {
		nnext = m_nodes[treeInd1].nnext();
		if (nnext == 1) {
			if (isend(treeInd1)) {
				return; // game ends
			}
			treeInd2 = next(treeInd1);
			if (is_next_ko_link(treeInd1)) {
				writeSGF0_link(fout, treeInd1, 0, "ko>");
				return; // ko link
			}
			if (check[treeInd2]) { // reached a link to an existing node
				writeSGF0_link(fout, treeInd1, 0, ">"); // write a node to represent a link
				return;
			}
			treeInd1 = treeInd2;
			writeSGF01(fout, treeInd1);
			check[treeInd1] = true;
		}
		else if (nnext == 0)
			return;
		else if (nnext > 1) {
			break; // reached downward fork
		}
	}

	// write downward branches
	for (i = 0; i < nnext; ++i) {
		fout << '(';
		if (isend(treeInd1))
			error("unkown error"); // game ends or ko link!
		treeInd2 = next(treeInd1, i);
		if (is_next_ko_link(treeInd1, i)) {
			writeSGF0_link(fout, treeInd1, i, "ko>"); // write a node to represent a link
		}
		else if (check[treeInd2]) { // reached a link to an existing node
			writeSGF0_link(fout, treeInd1, i, ">"); // write a node to represent a link
		}
		else
			writeSGF0(check, fout, treeInd2); // normal node
		fout << ')';
	}
	return;
}

// write one node to SGF file
inline void Tree::writeSGF01(ofstream &fout, Long_I treeInd1, const string &prefix)
{
	Char BW; // letter B or letter W
	Node & node = m_nodes[treeInd1];
	Int i, nnext;
	if (who(treeInd1) == Who::BLACK)
		BW = 'B';
	else if (who(treeInd1) == Who::WHITE)
		BW = 'W';
	else
		error("illegal stone color");

	fout << ";" << BW;
	if (node.ispass()) // pass
		fout << "[]";
	else
		fout << '[' << char('a' + node.x()) << char('a' + node.y()) << "]";

	// add node number to title
	fout << "N[" << prefix << "[" << treeInd1 << "\\]";
	// add score
	if (solved(treeInd1) || is_ko_node(treeInd1))
		fout << 0.5*score2(treeInd1);
	// end title
	fout << "]";

	// add green (black wins) or blue (white wins) mark
	if (is_ko_node(treeInd1))
		fout << "DO[]"; // brown
	else if (solution(treeInd1) == Sol::FORBIDDEN)
		fout << "BM[1]"; // red
	else if (winner(treeInd1) == Who::BLACK)
		fout << "TE[1]"; // green
	else if (winner(treeInd1) == Who::WHITE)
		fout << "IT[]"; // blue
	else if (winner(treeInd1) == Who::DRAW)
		error("TODO...");
	else if (winner(treeInd1) == Who::NONE)
		; // unsolved node
	else
		error("???");

	fout << "\n";
}

inline void Tree::writeSGF0_link(ofstream &fout, Long_I treeInd_from, Int_I forkInd, const string &prefix)
{
	Char BW; // letter B or letter W
	Node & node_from = m_nodes[treeInd_from];
	Int i, nnext;
	Who who = ::next(node_from.who());
	Long treeInd_to = next(treeInd_from, forkInd);
	if (who == Who::BLACK)
		BW = 'B';
	else if (who == Who::WHITE)
		BW = 'W';
	else
		error("illegal stone color");

	Move mov = nextMove(treeInd_from, forkInd);

	fout << ";" << BW;
	if (mov.ispass()) // pass
		fout << "[]";
	else
		fout << '[' << char('a' + mov.x()) << char('a' + mov.y()) << "]";

	// add node number to title
	fout << "N[" << prefix << "[" << treeInd_to << "\\]";
	// add score
	if (solved(treeInd_to))
		fout << 0.5*score2(treeInd_to);
	// end title
	fout << "]";

	// add green (black wins) or blue (white wins) mark
	Sol sol = solution(treeInd_to);
	Who winner = sol2winner(who, sol);
	if (is_ko_sol(sol))
		fout << "DO[]"; // brown
	else if (sol == Sol::FORBIDDEN)
		fout << "BM[1]"; // red
	else if (winner == Who::BLACK)
		fout << "TE[1]"; // green
	else if (winner == Who::WHITE)
		fout << "IT[]"; // blue
	else if (winner == Who::DRAW)
		error("TODO...");
	else if (winner == Who::NONE)
		; // unsolved node
	else
		error("???");

	fout << "\n";
}

inline Bool Tree::next_exist(Move mov, Long_I treeInd /*optional*/)
{
	Int i, j;
	vector<Move> moves;
	Long treeInd1 = def(treeInd);
	Node &node = m_nodes[treeInd1];
	for (i = node.nnext() - 1; i >= 0; --i) {
		if (nextMove(treeInd1, i) == mov)
			return true;
	}
	return false;
}

Int Tree::rand_move(Long_I treeInd /*optional*/)
{
	error("update this function from rand_smart_move()");
}

inline Int Tree::rand_smart_move(Long_I treeInd /*optional*/)
{
	Bool exist, exist_pass = false;
	Int i, j, ret, Nx = board_Nx(), Ny = board_Ny(), Nxy = Nx*Ny;
	Char x0, y0, x, y;
	VecInt xy;
	Long_I treeInd1 = def(treeInd);
	Node & node = m_nodes[treeInd1];
	
	BoardRef board = get_board(treeInd1);
	vector<Move> eat_pos;

	// consider placing that can eat stones
	board.eat_list(eat_pos, ::next(who(treeInd1)));
	for (i = 0; i < eat_pos.size(); ++i) {
		x = eat_pos[i].x(); y = eat_pos[i].y();
		// check existence
		if (next_exist(Move(x, y), treeInd1))
			continue;
		// check legal and number of removal
		ret = place(x, y, treeInd1);
		if (ret == -1)
			continue; // illegal move, no change
		else if (ret == 0)
			return 0; // new node created
		else if (ret == 1)
			return 1; // linked to a normal node, or clean ko node
		else if (ret == 2)
			return 4; // linked to a non-clean ko node
		else if (ret == -2)
			return 2; // linked to existing node, has ko
	}

	// random sequence of all grid points on board
	randPerm(xy, Nxy);

	// search xy for a new legal move (not in Node::m_next)
	for (i = 0; i < Nxy; ++i) {
		x = xy[i] % Nx; y = xy[i] / Nx;
		// check existence
		if (next_exist(Move(x, y), treeInd1))
			continue;
		// check legal and number of removal
		ret = check(x, y, treeInd1);
		if (ret < 0)
			continue;
		// check dumb eye filling
		if (board.is_dumb_eye_filling(x, y, ::next(who(treeInd1)))) {
			continue;
		}
		if (board.is_dumb_2eye_filling(x, y, ::next(who(treeInd1)))) {
			continue;
		}
		ret = place(x, y, treeInd1);
		if (ret == -1)
			continue; // illegal move, no change
		else if (ret == 0)
			return 0; // new node created
		else if (ret == 1)
			return 1;  // linked to an existing normal node or clean ko node
		else if (ret == -2)
			return 2; // ko link created
		else if (ret == 2)
			return 4; // linked to a non-clean ko node
	}

	// no non-dumb placing left, consider passing
	if (!next_exist(Move(Act::PASS), treeInd1)) {
		ret = pass(treeInd1);
		if (ret == -1)
			return -2; // double pass successful
		else if (ret == 0)
			return 0; // first pass
		else if (ret == -2) {
			return -3; // double pass not allowed
		}
		else if (ret == 1) {
			return 1; // linked to existing normal node or clean ko node
		}
		else if (ret == 2) {
			return 2; // ko link created
		}
		else if (ret == 3) {
			// linked to an(unclean) ko node
			return 4;
		}
		else
			error("unhandled case!");
	}

	// all leagl moves already exist
	return -1;
}

inline Int Tree::rand_game(Long_I treeInd, Bool_I out)
{
	Int i, ret;

	m_treeInd = treeInd;

	// debug: edit board here
	/*tree.place(0, 0); tree.place(1, 2);
	tree.place(2, 2); tree.place(1, 1);
	tree.place(0, 2); tree.place(0, 1);
	tree.place(2, 1); tree.place(1, 0);
	tree.pass();*/
	// end edit board

	if (out) disp_board();

	for (i = 1; i < 10000; ++i) {
		ret = rand_smart_move();
		if (ret == -1) {
			if (out) cout << "all moves exists\n\n\n";
			break;
		}
		if (ret == -2) {
			if (out) cout << "double pass !\n\n\n";
			break;
		}
		if (out) cout << "step " << i << endl;
		if (out) disp_board();
	}

	if (out) cout << "game over!" << "\n\n";

	solve_end();

	if (winner() == Who::BLACK) {
		if (out) cout << "black wins!";
		if (out) cout << "  (score: " << 0.5*score2() << ")\n\n";
	}
	else if (winner() == Who::WHITE) {
		if (out) cout << "white wins!";
		if (out) cout << "  (score: " << 0.5*score2() << ")\n\n";
	}
	else { // draw
		if (out) cout << "draw!\n\n";
	}
	if (out) writeSGF("randdom_game.sgf");

	return 0;
}

inline void Tree::solve_end(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);

	// TODO check if this is an end node
	if (!isend(treeInd1))
		error("Tree::solve_end(): unkown error!");

	calc_score(treeInd1); calc_sol(treeInd1);
}

Int Tree::solve(Long_I treeInd /*optional*/)
{
	Bool debug_stop = nnode() >= 314; // nnode() >= 2492 || treeInd1 == 1; // debug
	Bool save = false;
	Int i, move_ret, solve_ret, solve_ko_ret, child_treeInd;
	const Long treeInd1 = def(treeInd);
	Int child_sco2;
	Sol child_sol;

	// ko related
	Bool has_ko_link = false, has_ko_child = false;
	Int best_solvable_child_sco2 = -1;
	Sol best_solvable_child_sol = Sol::BAD;
	Int best_ko_child_sco2 = -1;

	// if already solved
	if (solution(treeInd1) != Sol::UNKNOWN) {
		if (is_ko_node(treeInd1)) {
			error("ko node should be solved by solve_ko() not solve()!");
		}
		else if (solution(treeInd1) == Sol::FORBIDDEN) {
			return 3;
		}
		return 0;
	}

	// enumerate children
	for (i = 0; i < 100000; ++i) {
		debug_stop = nnode() >= 314;
		//###########################################################
		move_ret = rand_smart_move(treeInd1);
		//###########################################################

		if (save) { // debug
			writeSGF("test.sgf");
			save = false;
		} // end debug

		if (move_ret == 0 || move_ret == 1 || move_ret == 3) {
			// 0: new node created
			// 1: linked to existing node
			child_treeInd = next(treeInd1, -1);
			// debug, display board
			if (move_ret == 0 || move_ret == 3) {
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
			}
			// end debug

			//###########################################################
			solve_ret = solve(child_treeInd);
			//###########################################################

			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug

			if (solve_ret == -1) {
				// is a ko child
				if (solution(child_treeInd) == Sol::KO_GOOD) {
					// is a good ko child
					set_score2(inv_score2(score2(child_treeInd)), treeInd1);
					set_solution(Sol::KO_BAD, treeInd1);
					if (resolve_ko(treeInd1)) {
						// not all downstream ko resolved
						return -1;
					}
					else {
						// all downstream ko resolved !
						push_clean_ko_node(treeInd1);
						return 2; // debug break point
					}
				}
				child_sco2 = score2(child_treeInd);
				if (best_ko_child_sco2 < child_sco2) {
					best_ko_child_sco2 = child_sco2;
				}
				has_ko_child = true;
				continue;
			}
			else if (solve_ret == 1) {
				// double passing solved two nodes
				return 0;
			}
			else if (solve_ret == 3) {
				// tried to solve forbidden node
				continue;
			}
			else if (solve_ret == 0 || solve_ret == 2) {
				child_sco2 = score2(child_treeInd);
				if (solve_ret == 0) {
					// successful!
					child_sol = solution(child_treeInd);
					// check if there is a color flip
				}
				else { // (solve_ret == 2)
					// a ko node with all ko links resolved
					// treat as a normal node
					child_sol = sco22sol(child_sco2, who(child_treeInd));
				}
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (best_solvable_child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						resolve_ko(treeInd1);
						return 0; // debug break point
					}
				}
			}
			else
				error("unhandled case!");
		}
		else if (move_ret == 4) {
			// linked to a non-clean ko node
			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug
			// break the link first
			child_treeInd = next(treeInd1, -1);
			solve_ko_ret = solve_ko(treeInd1, -1);

			if (solve_ko_ret == 0 || solve_ko_ret == 1) {
				// 0: solved to solvable node
				// 1: solved to clean ko node and pushed
				if (solve_ko_ret == 0) {
					child_sco2 = score2(child_treeInd);
					child_sol = solution(child_treeInd);
				}
				else {
					child_sco2 = clean_ko_node_sco2(child_treeInd);
					child_sol = clean_ko_node_sol(child_treeInd);
				}
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (best_solvable_child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						resolve_ko(treeInd1);
						return 0; // debug break point
					}
				}
				continue;
			}
			else if (solve_ko_ret == 2) {
				error("clean ko node should have catched by ");
			}
			else {
				error("unhandled case!");
			}
		}
		else if (move_ret == 2 || move_ret == -3) {
			// a ko link created
			Long child_treeInd = next(treeInd1, -1);
			if (solved(child_treeInd)) {
				// ko link target already solved
				child_sco2 = score2(child_treeInd);
				child_sol = solution(child_treeInd);
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (best_solvable_child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						resolve_ko(treeInd1);
						return 0; // debug break point
					}
				}
				continue;
			}
			else if (solution(child_treeInd) == Sol::UNKNOWN) {
				// ko link target not solved
				m_ko_link_from.push_back(treeInd1);
				m_ko_link_to.push_back(child_treeInd);
				has_ko_link = true;
				continue;
			}
			else
				error("unhandled case!");
		}
		else if (move_ret == -1) {
			// all children exist
			// no good child or good ko child is found
			if (!has_ko_link && !has_ko_child) {
				if (best_solvable_child_sco2 < 0) {
					// all children are forbidden
					set_solution(Sol::FORBIDDEN, treeInd1);
					return 3;
				}
				// all children solved
				if (best_solvable_child_sol == Sol::FAIR) {
					set_solution(Sol::FAIR, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				}
				else { // best == Sol::BAD
					set_solution(Sol::GOOD, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				}
				resolve_ko(treeInd1);
				return 0; // debug break point
			}
			// not all children solvable
			else if (has_ko_link && !has_ko_child) {
				// has ko link, no ko child
				if (best_solvable_child_sco2 < 0) {
					// forbidden node
					set_solution(Sol::FORBIDDEN, treeInd1);
					return 3;
				}
				set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
				calc_ko_sol(treeInd1);
				return -1;
			}
			else if (!has_ko_link && has_ko_child) {
				// has ko child, no ko link
				if (best_solvable_child_sco2 >= best_ko_child_sco2) {
					// trivial ko
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					calc_sol(treeInd1);
					resolve_ko(treeInd1);
					return 0;
				}
				else {
					// this is a ko node
					set_score2(inv_score2(best_ko_child_sco2), treeInd1);
					calc_ko_sol(treeInd1);
					if (resolve_ko(treeInd1)) {
						// still have unresolved ko link
						return -1;
					}
					else {
						// all downstream ko resolved !
						push_clean_ko_node(treeInd1);
						return 2; // debug break point
					}
				}
			}
			else { // (has_ko_link && has_ko_child)
				// has both ko child and ko link
				set_score2(inv_score2(MAX(best_solvable_child_sco2, best_ko_child_sco2)), treeInd1);
				calc_ko_sol(treeInd1);
				resolve_ko(treeInd1);
				return -1;
			}
		}
		else if (move_ret == -2) {
			// double passing caused game end.
			return 1;
		}
		else
			error("unhandled return!");
	}
	error("unkown error!"); return -1;
}

inline Int Tree::solve_ko(Long_I treeInd_from, Int_I forkInd)
{
	Int i, ret;
	Long treeInd_to = next(treeInd_from, forkInd);
	if (m_ko_link_from.size() != 0) {
		// not all ko links resolved
		error("unhandled case!");
	}

	// check clean ko node
	if (check_clean_ko_node(treeInd_to) >= 0) {
		// is a clean ko nod
		return 2;
	}

	// shift ko links to curren node
	shift_ko_links(treeInd_from, forkInd);

	// Now we can start solving!
	ret = solve_ko0(treeInd_to);
	return ret;
}

Int Tree::solve_ko0(Long_I treeInd1)
{
	Int i, solve_ret, move_ret;
	Int child_treeInd, child_sco2, Nchild = node(treeInd1).nnext();
	Int best_solvable_child_sco2 = -1, best_ko_child_sco2 = -1;
	Sol child_sol;
	Sol best_solvable_child_sol = Sol::BAD, best_ko_child_sol = Sol::BAD;
	Bool has_ko_link = false, has_ko_child = false, save = false;

	// check existing children
	for (i = 0; i < Nchild; ++i) {
		child_treeInd = node(treeInd1).next(i);
		if (is_ko_child(treeInd1, i)) {
			// found a ko child
			has_ko_child = true;
			solve_ret = solve_ko0(child_treeInd);
			if (solve_ret == 0) {
				// ko child is solved
				child_sco2 = score2(child_treeInd);
				child_sol = solution(child_treeInd);
				if (best_solvable_child_sco2 < child_sco2) {
					best_solvable_child_sco2 = child_sco2;
					best_solvable_child_sol = child_sol;
					if (child_sol == Sol::GOOD) {
						set_solution(Sol::BAD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						rm_clean_ko_node(treeInd1);
						return 0;
					}
				}
				continue;
			}
		}
		else if (node(treeInd1).is_next_ko_link(i)) {
			// found a ko link
			m_ko_link_from.push_back(treeInd1);
			m_ko_link_to.push_back(child_treeInd);
			has_ko_link = true;
			continue;
		}
		else {
			// found a solvable child (solved)
			child_sco2 = score2(child_treeInd);
			child_sol = solution(child_treeInd);
			if (best_solvable_child_sco2 < child_sco2) {
				best_solvable_child_sco2 = child_sco2;
				best_solvable_child_sol = child_sol;
				if (child_sol == Sol::GOOD) {
					set_solution(Sol::BAD, treeInd1);
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					rm_clean_ko_node(treeInd1);
					return 0;
				}
			}
		}
	}

	if (solution(treeInd1) == Sol::KO_BAD) {
		for (i = 0; i < 1000; ++i) {
			// not all children checked
			// try to create other children
			move_ret = rand_smart_move(treeInd1);
			if (save) { // debug
				writeSGF("test.sgf");
				save = false;
			} // end debug
			if (move_ret == 0 || move_ret == 1) {
				// 0: successful and new node created
				// 1: linked to existing node and no ko
				// begin debug
				cout << "largest treeInd = " << max_treeInd() << endl;
				disp_board(max_treeInd()); cout << "\n\n" << endl;
				cout << ""; // debug break point
				// end debug
				child_treeInd = next(treeInd1, -1);
				solve_ret = solve(child_treeInd);
				if (save) { // debug
					writeSGF("test.sgf");
					save = false;
				} // end debug
				if (solve_ret == 0) {
					child_sco2 = score2(child_treeInd);
					child_sol = solution(child_treeInd);
					if (best_solvable_child_sco2 < child_sco2) {
						best_solvable_child_sco2 = child_sco2;
						best_solvable_child_sol = child_sol;
						if (child_sol == Sol::GOOD) {
							set_solution(Sol::BAD, treeInd1);
							set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
							rm_clean_ko_node(treeInd1);
							return 0;
						}
					}
					continue;
				}
				else {
					error("unhandled case");
				}
			}
			else if (move_ret == 2 || move_ret == -3) {
				// 2: linked to existing node and has ko
				// -3: double pass not allowed and ko link created
				// a ko link created
				Long child_treeInd = next(treeInd1, -1);
				if (solved(child_treeInd)) {
					// ko link target already solved
					child_sco2 = score2(child_treeInd);
					child_sol = solution(child_treeInd);
					if (best_solvable_child_sco2 < child_sco2) {
						best_solvable_child_sco2 = child_sco2;
						best_solvable_child_sol = child_sol;
						if (best_solvable_child_sol == Sol::GOOD) {
							set_solution(Sol::BAD, treeInd1);
							set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
							resolve_ko(treeInd1);
							rm_clean_ko_node(treeInd1);
							return 0; // debug break point
						}
					}
					continue;
				}
				else if (solution(child_treeInd) == Sol::UNKNOWN) {
					// ko link target not solved
					m_ko_link_from.push_back(treeInd1);
					m_ko_link_to.push_back(child_treeInd);
					has_ko_link = true;
					continue;
				}
				else
					error("unhandled case!");
			}
			else if (move_ret == -2) {
				// passed and game ends
				error("unhandled case!");
			}
			else if (move_ret == -1) {
				// all children exist
				// no good child or good ko child is found
				if (!has_ko_link && !has_ko_child) {
					if (best_solvable_child_sco2 < 0) {
						// all children are forbidden
						set_solution(Sol::FORBIDDEN, treeInd1);
						return 3;
					}
					// all children solved
					if (best_solvable_child_sol == Sol::FAIR) {
						set_solution(Sol::FAIR, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					}
					else { // best == Sol::BAD
						set_solution(Sol::GOOD, treeInd1);
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					}
					resolve_ko(treeInd1);
					rm_clean_ko_node(treeInd1);
					return 0; // debug break point
				}
				// not all children solvable
				else if (has_ko_link && !has_ko_child) {
					// has ko link, no ko child
					if (best_solvable_child_sco2 < 0) {
						// forbidden node
						set_solution(Sol::FORBIDDEN, treeInd1);
						return 3;
					}
					set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
					calc_ko_sol(treeInd1);
					return -1;
				}
				else if (!has_ko_link && has_ko_child) {
					// has ko child, no ko link
					if (best_solvable_child_sco2 >= best_ko_child_sco2) {
						// trivial ko
						set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
						calc_sol(treeInd1);
						resolve_ko(treeInd1);
						rm_clean_ko_node(treeInd1);
						return 0;
					}
					else {
						// this is a ko node
						set_score2(inv_score2(best_ko_child_sco2), treeInd1);
						calc_ko_sol(treeInd1);
						if (resolve_ko(treeInd1)) {
							// still have unresolved ko link
							return -1;
						}
						else {
							// all downstream ko resolved !
							push_clean_ko_node(treeInd1);
							return 2; // debug break point
						}
					}
				}
				else { // (has_ko_link && has_ko_child)
					   // has both ko child and ko link
					set_score2(inv_score2(MAX(best_solvable_child_sco2, best_ko_child_sco2)), treeInd1);
					calc_ko_sol(treeInd1);
					resolve_ko(treeInd1);
					return -1;
				}
			}
		}
	}

	// all children are checked
	// all children exist
	// no good child or good ko child is found
	if (!has_ko_link && !has_ko_child) {
		if (best_solvable_child_sco2 < 0) {
			// all children are forbidden
			set_solution(Sol::FORBIDDEN, treeInd1);
			return 3;
		}
		// all children solved
		if (best_solvable_child_sol == Sol::FAIR) {
			set_solution(Sol::FAIR, treeInd1);
			set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
		}
		else { // best == Sol::BAD
			set_solution(Sol::GOOD, treeInd1);
			set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
		}
		resolve_ko(treeInd1);
		rm_clean_ko_node(treeInd1);
		return 0; // debug break point
	}
	// not all children solvable
	if (has_ko_link && !has_ko_child) {
		// has ko link, no ko child
		if (best_solvable_child_sco2 < 0) {
			error("impossible case!");
		}
		set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
		calc_ko_sol(treeInd1);
		return 1;
	}
	else if (!has_ko_link && has_ko_child) {
		// has ko child, no ko link
		if (best_solvable_child_sco2 >= best_ko_child_sco2) {
			// trivial ko
			set_score2(inv_score2(best_solvable_child_sco2), treeInd1);
			calc_sol(treeInd1);
			resolve_ko(treeInd1);
			rm_clean_ko_node(treeInd1);
			return 0;
		}
		else {
			// this is a ko node
			set_score2(inv_score2(best_ko_child_sco2), treeInd1);
			calc_ko_sol(treeInd1);
			if (resolve_ko(treeInd1)) {
				// still have unresolved ko link
				return 1;
			}
			else {
				// all downstream ko resolved !
				push_clean_ko_node(treeInd1);
				return 1; // debug break point
			}
		}
	}
	else { // (has_ko_link && has_ko_child)
		   // has both ko child and ko link
		set_score2(inv_score2(MAX(best_solvable_child_sco2, best_ko_child_sco2)), treeInd1);
		calc_ko_sol(treeInd1);
		resolve_ko(treeInd1);
		return 1;
	}
}

Who Tree::winner(Long treeInd) const
{
	Long treeInd1 = def(treeInd);
	Sol sol = solution(treeInd1);
	Who player = who(treeInd1);
	return sol2winner(player, sol);
}

void Tree::calc_sol(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);
	Sol sol = sco22sol(score2(treeInd1), who(treeInd1));
	set_solution(sol, treeInd1);
}

void Tree::calc_ko_sol(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);
	Sol sol = sco22sol(score2(treeInd1), who(treeInd1));
	set_solution(sol2ko_sol(sol), treeInd1);
}

inline Int Tree::score2(Long_I treeInd) const
{
	Int sco2 = node(def(treeInd)).score2();
	if (sco2 < 0 || sco2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	return sco2;
}

inline void Tree::set_score2(Int score2, Long_I treeInd)
{
	if (score2 < 0 || score2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	node(def(treeInd)).set_sco2(score2);
}

inline Sol Tree::solution(Long_I treeInd) const
{
	return node(def(treeInd)).solution();
}

inline void Tree::set_solution(Sol_I sol, Long_I treeInd)
{
	node(def(treeInd)).set_solution(sol);
}

inline Bool Tree::solved(Long_I treeInd) const
{
	Sol sol = solution(def(treeInd));
	if (sol == Sol::GOOD || sol == Sol::BAD || sol == Sol::FAIR)
		return true;
	return false;
}

void Tree::calc_score(Long_I treeInd) // calculate territory and save to m_territory2
{
	Long treeInd1 = def(treeInd);
	Int sco2 = get_board(treeInd1).calc_territory2(who(treeInd1));
	set_score2(sco2, treeInd1);
}
