#pragma once
#include "node.h"
#include "pool.h"
#include "boardref.h"
#include "SLISC/search.h"
#include "SLISC/input.h"

// game tree
// a tree index is an index for m_nodes (treeInd), this index should never change for the same node
// if any node has nlast() > 1, it creates an "upward fork"
class Tree
{
private:
	// === data members ===
	vector<Node> m_nodes;
	Pool m_pool;

	// unresolved ko links
	vector<Linkp> m_unreso_ko_links; // debug

	// clean ko nodes
	vector<Long> m_clean_ko_node;
	vector<Int> m_clean_ko_node_sco2;
	vector<Sol> m_clean_ko_node_sol;

	// links
	vector<Link> m_links;

public:
	// input
	slisc::Input inp;

	// === constructors ===
	Tree();

	// === general methods ===

	Who who(Long_I treeInd) const; // who played the node

	// who relative to config, not situation (difference when color flip exists)
	Who who_config(Long_I treeInd) const;

	Long nnode() const { return m_nodes.size(); } // return number of nodes in the tree

	Long max_treeInd() const { return m_nodes.size() - 1; }; // maximum treeInd

	void disp_board(Long_I treeInd) const; // display board

	BoardRef get_board(Long_I treeInd) const; // return the board

	Int nlast(Long_I treeInd) const; // number of parents

	// return a pointer of a parent link
	Linkp last(Long_I treeInd, Int_I forkInd = 0) const;

	// return a reference of the last node of a node
	const Node & lastNode(Long_I treeInd, Int_I forkInd = 0) const;

	// return a pointer of a child link
	Linkp next(Long_I treeInd, Int_I forkInd = 0) const;

	// return a reference of the next node of a node
	// forkInd can be negative
	const Node & nextNode(Long_I treeInd, Int_I forkInd = 0) const;

	// return a reference of the next node of a node
	Node & nextNode(Long_I treeInd, Int_I forkInd = 0);

	// check if the next node have this move already
	Bool nextMove_exist(Move mov, Long_I treeInd) const;

	inline Bool isend(Long_I treeInd) const; // if a node is an end node

	// create simple ko link
	void link(LnType_I type, Long_I treeInd_from, Long_I treeInd_to, Move_I move);

	// create trans ko link
	void link(LnType_I type, Long_I treeInd_from, Long_I treeInd_to, Move_I move, Trans_I trans);

	// unlink two nodes
	Linkp unlink(Long_I treeInd_from, Int_I forkInd);

	// relink to nodes
	void relink(Linkp_I plink);

	// check if node(treeInd_from) is an upstream node of node(treeInd_to)
	// ko links are ignored
	// return 0 if not linked
	// return 1 if linked (treeInd1 != treeInd2)
	// return 2 if treeInd1 == treeInd2
	inline Int islinked(Long_I treeInd1, Long_I treeInd2) const;
	
	// ======== ko related methods =======================

	Bool is_ko_node(Long_I treeInd) const;

	Bool is_ko_child(Long_I treeInd, Int_I forkInd = 0) const;

	// check if same board already exists in the Pool and decide if it is a Ko
	// 'nodes[treeInd]' will produce 'config' in the next move
	// if board already exists, output tree index of the same board
	// return 0 if this is a new configuration
	// return -1 if situation exists but not a ko
	// return -2 if situation exists and is a ko
	// return -3 if this is a new situation of an existing configuration
	Int check_ko(Int_O search_ret, Long_O treeInd_match, Long_O orderInd,
		Long_I treeInd, Config_I config, Bool_I flip) const;

	// find all downstream ko links of a node through ko children
	// this is a recursive function
	void ko_links(vector<Linkp> & pLinks, Long_I treeInd) const;

	// push a clean ko node to record
	void push_clean_ko_node(Long_I treeInd1);

	// try to remove clean ko node record
	// to nothing if not a clean ko node
	// return 0 if removed
	// return 1 if nothing is done
	Int rm_clean_ko_node(Long_I treeInd);

	// check if there is no downstream ko links to upstream (not including current node)
	// downstream ko links are searched through ko children.
	Bool is_new_clean_ko_node(Long treeInd) const;

	// if the node is a clean ko node
	// return -1 if not clean
	// return index to m_clean_ko_node[] if clean
	Int check_clean_ko_node(Long_I treeInd) const;

	// get the score of a clean ko node
	// don't trust the score2 function
	// 'which' is the return from check_clean_ko_node()
	Int clean_ko_node_sco2(Int_I cleanInd) const;

	Sol clean_ko_node_sol(Int_I cleanInd) const;

	// ========= move related methods ==================


	MovRet pass(Long_O child_treeInd, Long_I treeInd);

	// check if a placing is legal, or how many stones will be dead
	// same check already exists for place()
	// Ko is not considered!
	// if legal, return the number of stones that can be removed ( >= 0)
	// return -1 if occupied, do nothing
	// return -2 if no qi, do nothing
	Int check(Char_I x, Char_I y, Long_I treeInd);

	// return 0 if new node created
	// return 1 if linked to a non-ko (solved/forbidden/unkown) node
	// return 2 if linked to an unclean ko node
	// return 3 if linked to a clean ko node
	// return -1 if illegal (nothing changes)
	// return -2 if a ko link is created
	// already has bound checking
	MovRet place(Long_O child_treeInd, Char_I x, Char_I y, Long_I treeInd);

	// smarter random move for a node
	// will not do a dumb move (dumb eye filling or dumb big eye filling)
	// return 0 if successful and new node created
	// return 1 if linked to a normal node
	// return 2 if a ko link is created
	// return 3 if linked to a clean ko node
	// return 4 if linked to a unclean ko node
	// return -1 if all legal moves already exists
	// return -2 if passed and game ends
	// return -3 if double pass not allowed and ko link created
	MovRet rand_smart_move(Long_O child_treeInd, Long_I treeInd);

	// prompt user for a move
	// returns are the same as auto_smart_move()
	MovRet prompt_move(Long_O child_treeInd, Long_I treeInd);

	// randomly play to the end of the game from a node
	// m_treeInd will be changed to the end node!
	// the end node will have the correct Node::m_win
	// using rand_move1() currently
	// set "out = true" for output to terminal and test.sgf
	inline Int rand_game(Long_I treeInd, Bool_I out = false);

	// =========== SGF related ============

	// write the whole tree to SGF file
	inline void writeSGF(const string &name) const;

	// internal function called by writeSGF()
	inline void writeSGF0(VecBool &check, ofstream &fout, Long_I treeInd_from, Int_I forkInd, Trans_I trans) const;

	// internal function called by writeSGF0()
	inline void writeSGF01(ofstream &fout, Long_I treeInd_from, Int_I forkInd, Trans_I trans) const;

	inline void writeSGF0_link(ofstream &fout, Long_I treeInd_from, Int_I forkInd, const string &prefix, Trans_I trans) const;

	// ======== solution related ============

	// the winner of a node
	Who winner(Long treeInd) const;

	// calculate and update score for a node
	void calc_score(Long_I treeInd);

	// update solution for a node based on scores and komi
	void calc_sol(Long_I treeInd);

	// update solution for a ko node based on scores and komi
	void calc_ko_sol(Long_I treeInd);

	// stored doubled score of a node (might not be up to date)
	Int score2(Long_I treeInd) const;

	// set score of a node
	void set_score2(Int score2, Long_I treeInd);

	// stored solution of a node (might not be up to date)
	Sol solution(Long_I treeInd) const;

	// set solution of a node
	void set_solution(Sol_I sol, Long_I treeInd);

	// if a node is solved (GOOD/BAD/FAIR)
	Bool solved(Long_I treeInd) const;

	Sol & best_child_sol(Long_I treeInd) const;

	Int & best_child_sco2(Long_I treeInd) const;

	// try to resolve all ko links to a node
	// return the number of unresolved links
	Int resolve_ko(Long_I treeInd_to);

	// try to resolve a ko link
	// do nothing if it's already resolved
	// return the number of unresolved links
	void resolve_ko_record(Long_I treeInd_from, Long_I treeInd_to);

	// shift [resolved downstream ko links to upstream] of a node to target it (treeInd_to)
	// first change those ko links to normal links
	// then change normal links from all parents of the node to ko links
	// do nothing if no qualified ko link found
	// return the number of ko links shifted
	Int shift_ko_links(Long_I treeInd_from, Int_I forkInd);

	void solve_end(Long_I treeInd); // solve a bottom node

	// analyse who has winning strategy for a node
	// might m_treeInd be changed? it shouldn't
	// this is a recursive function
	// using rand_smart_move() to generate moves until a better one is available
	// assuming there is no branch after treeInd for now
	// return 0 if successful
	// return 1 if double pass (two nodes linked by the first pass both solved)
	// return -1 if is a unclean ko node
	// return 2 if is a clean ko node
	// return 3 if is a forbidden node
	Int solve(Long_I treeInd);

	~Tree() {}
};

Who Tree::who(Long_I treeInd) const // who played the node
{
	return m_nodes[treeInd].who();
}

BoardRef Tree::get_board(Long_I treeInd) const
{
	const Config & config = m_pool(m_nodes[treeInd].poolInd());
	const Trans & trans = m_nodes[treeInd].trans();
	BoardRef board_ref(config, trans);
	return board_ref;
}

inline Int Tree::nlast(Long_I treeInd) const
{
	return m_nodes[treeInd].nlast();
}

// create 0-th node: empty board
Tree::Tree()
{
	inp.openfile("inp.txt");
	Board board; board.init();
	m_pool.push(board, -3, 0, Who::NONE, 0);
	m_nodes.emplace_back(); m_nodes.back().init();
}

inline Bool Tree::isend(Long_I treeInd) const
{
	if (m_nodes[treeInd].next()->isend())
		return true;
	return false;
}

inline Linkp Tree::last(Long_I treeInd, Int_I forkInd) const
{
	return m_nodes[treeInd].last(forkInd);
}

const Node & Tree::lastNode(Long_I treeInd, Int_I forkInd) const // return a reference of the last node of a node
{
	return m_nodes[last(treeInd, forkInd)->from()];
}

inline Linkp Tree::next(Long_I treeInd, Int_I forkInd) const
{
	return m_nodes[treeInd].next(forkInd);
}

inline const Node & Tree::nextNode(Long_I treeInd, Int_I forkInd) const
{
	return m_nodes[next(treeInd, forkInd)->to()];
}

Node & Tree::nextNode(Long_I treeInd, Int_I forkInd) // return a reference of the next node of a node
{
	return m_nodes[next(treeInd, forkInd)->to()];
}

Who Tree::who_config(Long_I treeInd) const
{
	if (treeInd == 0)
		error("unhandled case!");
	if (get_board(treeInd).trans().flip())
		return ::next(who(treeInd));
	return who(treeInd);
}

inline void Tree::disp_board(Long_I treeInd) const
{
	Char x, y;
	Int i;
	const Node & node = m_nodes[treeInd];
	for (i = 0; i < nlast(treeInd); ++i) {
		cout << i << ": ";
		if (node.isinit())
			cout << "(empty)" << endl;
		else {
			if (who(treeInd) == Who::BLACK)
				cout << "(black ";
			else if (who(treeInd) == Who::WHITE)
				cout << "(white ";
			else
				error("Tree:disp(): illegal side name!");

			if (node.last(i)->ispass())
				cout << "pass)" << endl;
			else if (node.last(i)->isedit())
				cout << "edit board)" << endl;
			else {
				cout << "[" << Int(node.last(i)->x()) << "," << Int(node.last(i)->y()) << "])" << endl;
			}
		}
	}

	get_board(treeInd).disp();
	cout << '\n' << endl;
};

inline MovRet Tree::pass(Long_O child_treeInd, Long_I treeInd)
{
	Int i, ret;
	Long ko_treeInd;
	const BoardRef board = get_board(treeInd);
	Node & node = m_nodes[treeInd];

	// check double pass
	for (i = 0; i < node.nlast(); ++i) {
		if (node.last(i)->move().ispass()) {
			// double passed!
			if (board.is_game_end()) {
				// two situations have the same scores and solutions!
				calc_score(treeInd);
				calc_sol(treeInd);
				set_score2(inv_score2(node.score2()), last(treeInd, i)->from());
				set_solution(inv_sol(node.solution()), last(treeInd, i)->from());
				return MovRet::DB_PAS_END;
			}
			else {
				// game not ended, build a ko link
				child_treeInd = last(treeInd, i)->from();
				Trans trans = lastNode(treeInd, i).trans() - node.trans();
				if (is_one(trans)) {
					link(LnType::KO_S, treeInd, child_treeInd, Move(Act::PASS));
				}
				else {
					link(LnType::KO_T, treeInd, child_treeInd, Move(Act::PASS), trans);
				}
				return MovRet::DB_PAS_KO_LN;
			}
			break;
		}
	}

	// check ko
	Int search_ret;
	Long orderInd, treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd, board.config(), board.trans().flip());

	if (ret == -3) {
		// configuration exists, new situation
		m_nodes.emplace_back();
		m_nodes.back().set(::next(who(treeInd)), m_pool.poolInd(orderInd), board.trans());
		child_treeInd = max_treeInd();
		link(LnType::SIMPLE, treeInd, child_treeInd, Move(Act::PASS));
		m_pool.link(orderInd, ::next(who_config(treeInd)), child_treeInd);
		return MovRet::NEW_ND;
	}
	else if (ret == -1) {
		// situation exists, not a ko
		child_treeInd = treeInd_found;
		Trans trans = m_nodes[child_treeInd].trans() - node.trans();
		if (is_one(trans)) {
			link(LnType::SIMPLE, treeInd, child_treeInd, Move(Act::PASS));
		}
		else {
			link(LnType::TRANS, treeInd, child_treeInd, Move(Act::PASS), trans);
		}
			
		if (is_ko_node(child_treeInd)) {
			// linked to a ko node
			if (check_clean_ko_node(child_treeInd) >= 0) {
				// linked to a clean ko node
				return MovRet::LN_CLN_KO_ND;
			}
			else {
				// linked to an unclean ko node
				return MovRet::LN_UCLN_KO_ND;
			}
		}
		else {
			// linked to a non-ko (solved/forbidden/unkown) node
			return MovRet::LN_NKO_ND;
		}
	}
	else if (ret == -2) {
		// situation exists, is a ko
		child_treeInd = treeInd_found;
		Trans trans = m_nodes[child_treeInd].trans() - node.trans();
		if (is_one(trans)) {
			link(LnType::KO_S, treeInd, child_treeInd, Move(Act::PASS));
		}
		else {
			link(LnType::KO_T, treeInd, child_treeInd, Move(Act::PASS), trans);
		}
		return MovRet::KO_LN;
	}
	else if (ret == 0) { // configuration does not exist
		error("impossible case, configuration must exist!");
	}
	else
		error("unhancled case!");
}

inline Int Tree::check(Char_I x, Char_I y, Long_I treeInd)
{
	Int ret;
	// first move
	if (treeInd == 0)
		return 0;
	// update board and check illegal move (Ko no checked!)
	return get_board(treeInd).check(x, y, ::next(who(treeInd)));
}

inline MovRet Tree::place(Long_O child_treeInd, Char_I x, Char_I y, Long_I treeInd)
{
	Int ret;
	Board board;
	board = get_board(treeInd);

	Node & node = m_nodes[treeInd];
	Who who = Tree::who(treeInd), next_who = ::next(who);

	// first move
	if (treeInd == 0) {
		board.place(x, y, Who::BLACK);
		m_pool.push(board, 1, -1, Who::BLACK, 1);
		m_nodes.emplace_back();
		m_nodes.back().set(Who::BLACK, m_pool.size() - 1, board.trans());
		child_treeInd = max_treeInd();
		link(LnType::SIMPLE ,treeInd, child_treeInd, Move(x, y));
		return MovRet::NEW_ND;
	}

	// update board and check illegal move (Ko no checked!)
	if (board.place(x, y, next_who))
		return MovRet::ILLEGAL;

	// check Ko
	Int search_ret;
	Long orderInd;
	Long treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd, board.config(), board.trans().flip());
	if (ret < 0) { // board already exists
		child_treeInd = treeInd_found;
		if (ret == -1) {
			// not a ko
			Trans trans = m_nodes[treeInd_found].trans() - board.trans();
			if (is_one(trans)) {
				link(LnType::SIMPLE, treeInd, treeInd_found, Move(x, y));
			}
			else {
				link(LnType::TRANS, treeInd, treeInd_found, Move(x, y), trans);
			}
			if (is_ko_node(treeInd_found)) {
				// linked to a ko node
				if (check_clean_ko_node(treeInd_found) >= 0) {
					// linked to a clean ko node
					return MovRet::LN_CLN_KO_ND;
				}
				else {
					// linked to an unclean ko node
					return MovRet::LN_UCLN_KO_ND;
				}
			}
			else {
				// linked to a non-ko (solved/forbidden/unkown) node
				return MovRet::LN_NKO_ND;
			}
		}
		else if (ret == -2) {
			// is a ko
			Trans trans = m_nodes[treeInd_found].trans() - board.trans();
			if (is_one(trans)) {
				link(LnType::KO_S, treeInd, treeInd_found, Move(x, y));
			}
			else {
				link(LnType::KO_T, treeInd, treeInd_found, Move(x, y), trans);
			}
			return MovRet::KO_LN;
		}
	}

	// new situation
	m_nodes.emplace_back(); // add Node to tree
	child_treeInd = max_treeInd();

	if (ret == -3) {
		// configuration exists
		Who next_who_config;
		if (board.trans().flip())
			next_who_config = ::next(next_who);
		else
			next_who_config = next_who;
		m_pool.link(orderInd, next_who_config, child_treeInd);
		m_nodes.back().set(next_who, m_pool.poolInd(orderInd), board.trans());
	}
	else { // configuration does not exist
		m_pool.push(board, search_ret, orderInd, next_who, child_treeInd);
		m_nodes.back().set(next_who, m_pool.size() - 1, board.trans());
	}

	link(LnType::SIMPLE, treeInd, child_treeInd, Move(x, y));
	return MovRet::NEW_ND;
}

inline void Tree::link(LnType_I type, Long_I treeInd_from, Long_I treeInd_to, Move_I move)
{
	Linkp plink = Linkp::newlink();
	plink->link(type, treeInd_from, treeInd_to, move);
	m_nodes[treeInd_from].push_next(plink);
	m_nodes[treeInd_to].push_last(plink);
	if (type == LnType::KO_S)
		m_unreso_ko_links.push_back(plink); // debug
}

inline void Tree::link(LnType_I type, Long_I treeInd_from, Long_I treeInd_to, Move_I move, Trans_I trans)
{
	Linkp plink = Linkp::newlink();
	plink->link(type, treeInd_from, treeInd_to, move, trans);
	m_nodes[treeInd_from].push_next(plink);
	m_nodes[treeInd_to].push_last(plink);
	if (type == LnType::KO_T)
		m_unreso_ko_links.push_back(plink); // debug
}

inline Linkp Tree::unlink(Long_I treeInd_from, Int_I forkInd)
{
	Node & node = m_nodes[treeInd_from];
	Linkp plink = node.next(forkInd);
	Node & next_node = m_nodes[plink->to()];
	node.delete_next(forkInd);
	next_node.delete_last(next_node.last_forkInd(plink));
	return plink;
}

inline void Tree::relink(Linkp_I plink)
{
	Node & node_from = m_nodes[plink->from()];
	Node & node_to = m_nodes[plink->to()];
	node_from.push_next(plink);
	node_to.push_last(plink);
}

// this is a recursive function
inline Int Tree::islinked(Long_I treeInd_from, Long_I treeInd_to) const
{
	Long i, treeInd;
	if (treeInd_from == treeInd_to) {
		return 2;
	}
	treeInd = treeInd_to;
	for (i = 0; i < 10000; ++i) {
		if (treeInd == treeInd_from)
			return 1; // found treeInd_from
		if (m_nodes[treeInd].nlast() > 1)
			break; // multiple upward fork
		if (treeInd == 0)
			return 0; // reached top of tree
		if (last(treeInd, 0)->isko())
			error("lonely parent!");
		treeInd = last(treeInd, 0)->from(); // single line, go up
	}
	for (i = 0; i < m_nodes[treeInd].nlast(); ++i) {
		if (last(treeInd, i)->isko()) {
			continue;
		}
		else {
			Int ret = islinked(treeInd_from, last(treeInd, i)->from());
			if (ret == 1 || ret == 2) {
				return 1;
			}
		}
	}
	return false;
}

inline Int Tree::check_ko(Int_O search_ret, Long_O treeInd_match, Long_O orderInd,
	Long_I treeInd, Config_I config, Bool_I flip) const
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
		if (islinked(treeInd_match, treeInd) == 1)
			return -2; // Ko!
		return -1; // no Ko
	}
	poolInd = -1;
	return 0;
}

inline void Tree::push_clean_ko_node(Long_I treeInd1)
{
	m_clean_ko_node.push_back(treeInd1);
	m_clean_ko_node_sco2.push_back(score2(treeInd1));
	m_clean_ko_node_sol.push_back(solution(treeInd1));
}

Bool Tree::is_new_clean_ko_node(Long treeInd) const
{
	Int i;
	const Node & node = m_nodes[treeInd];
	vector<Linkp> plinks;
	// ko links from downstream to upstream converts to normal links
	ko_links(plinks, treeInd);
	for (i = plinks.size() - 1; i >= 0; --i) {
		if (islinked(plinks[i]->to(), treeInd) == 1) {
			return false;
		}
	}
	return true;
}

inline void Tree::ko_links(vector<Linkp> & plinks, Long_I treeInd) const
{
	Int i;
	Long ko_treeInd;
	const Node & node = m_nodes[treeInd];
	for (i = 0; i < node.nnext(); ++i) {
		if (is_ko_child(treeInd, i)) {
			// found a ko child
			ko_links(plinks, node.next(i)->to());
		}
		else if (node.next(i)->isko()) {
			// found a ko link
			plinks.push_back(node.next(i));
		}
		// ignoring normal child
	}
}

Int Tree::check_clean_ko_node(Long_I treeInd) const
{
	Int i, N = m_clean_ko_node.size();
	for (i = 0; i < N; ++i) {
		if (m_clean_ko_node[i] == treeInd)
			return i;
	}
	return -1;
}

inline Int Tree::rm_clean_ko_node(Long_I treeInd)
{
	Int cleanInd = check_clean_ko_node(treeInd);
	if (cleanInd >= 0) {
		m_clean_ko_node.erase(m_clean_ko_node.begin() + cleanInd);
		m_clean_ko_node_sco2.erase(m_clean_ko_node_sco2.begin() + cleanInd);
		m_clean_ko_node_sol.erase(m_clean_ko_node_sol.begin() + cleanInd);
		return 0;
	}
	return 1;
}

Int Tree::clean_ko_node_sco2(Int_I cleanInd) const
{
	return m_clean_ko_node_sco2[cleanInd];
}

Sol Tree::clean_ko_node_sol(Int_I cleanInd) const
{
	return m_clean_ko_node_sol[cleanInd];
}

inline void Tree::writeSGF(const string &name) const
{
	Int i;
	Char Nx = board_Nx(), Ny = board_Ny();
	VecBool check(nnode(), false);
	ofstream fout(name);
	fout << "(\n";
	fout << "  ;GM[1]FF[4]CA[UTF-8]AP[]KM[" << 0.5*komi2() << "]";

	// board size
	if (Nx == Ny)
		fout << "SZ[" << Int(Nx) << "]";
	else
		fout << "SZ[" << Int(Nx) << ":" << Int(Ny) << "]";
	fout << "DT[]" << endl;

	for (i = 0; i < m_nodes[0].nnext(); ++i) {
		fout << '(';
		writeSGF0(check, fout, 0, i, Trans());
		fout << ')';
	}
	
	if (count(check) != nnode() - 1)
		error("writeSGF() nodes number does not match!");

	fout << ")\n";
	fout.close();
}

// return max node index written
// nodes should be written in the order of treeIndex
// recursively write the tree
inline void Tree::writeSGF0(VecBool &check, ofstream &fout, Long_I treeInd_from, Int_I forkInd, Trans_I trans0) const
{
	using slisc::lookupInt;
	Int i, nnext;
	Long treeInd;
	Linkp plink;
	Trans trans = trans0;

	// check trans link
	plink = next(treeInd_from, forkInd);

	// write one node
	writeSGF01(fout, treeInd_from, forkInd, trans);
	if (plink->istrans()) {
		trans -= plink->trans();
	}

	treeInd = next(treeInd_from, forkInd)->to();
	check[treeInd] = true;

	// go down the branch if no fork
	for (i = 0; i < 10000; ++i) {
		nnext = m_nodes[treeInd].nnext();
		if (nnext == 1) {
			if (isend(treeInd)) {
				// game ends
				return;
			}
			plink = next(treeInd, 0);
			if (plink->isko()) {
				// ko link
				writeSGF0_link(fout, treeInd, 0, "ko>", trans);
				return;
			}
			if (check[plink->to()]) {
				// normal link
				writeSGF0_link(fout, treeInd, 0, ">", trans); // write a node to represent a link
				return;
			}
			// a child
			writeSGF01(fout, treeInd, 0, trans);
			if (plink->istrans()) {
				trans -= plink->trans();
			}
			check[plink->to()] = true;
			treeInd = plink->to();
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
		if (isend(treeInd))
			error("unkown error!");
		plink = next(treeInd, i);
		if (plink->isko()) {
			writeSGF0_link(fout, treeInd, i, "ko>", trans); // write a node to represent a link
		}
		else if (check[plink->to()]) { // reached a link to an existing node
			writeSGF0_link(fout, treeInd, i, ">", trans); // write a node to represent a link
		}
		else
			writeSGF0(check, fout, treeInd, i, trans); // normal node
		fout << ')';
	}
	return;
}

// write one node to SGF file
inline void Tree::writeSGF01(ofstream &fout, Long_I treeInd_from, Int_I forkInd, Trans_I trans) const
{
	Char BW, x, y; // letter B or letter W
	Who who;
	Linkp plink = next(treeInd_from, forkInd);
	Long treeInd = plink->to();

	if (trans.flip())
		who = Tree::who(treeInd_from);
	else
		who = ::next(Tree::who(treeInd_from));

	if (who == Who::BLACK)
		BW = 'B';
	else if (who == Who::WHITE)
		BW = 'W';
	else
		error("illegal stone color");

	fout << ";" << BW;
	if (plink->ispass()) // pass
		fout << "[]";
	else {
		x = plink->x(); y = plink->y();
		transf(x, y, trans.rot());
		fout << '[' << char('a' + x) << char('a' + y) << "]";
	}

	// add node number to title
	fout << "N[" << "[" << treeInd << "\\]";
	// add score
	if (solved(treeInd) || is_ko_node(treeInd))
		fout << 0.5*score2(treeInd);
	// end title
	fout << "]";

	// add green (black wins) or blue (white wins) mark
	if (is_ko_node(treeInd))
		fout << "DO[]"; // brown
	else if (solution(treeInd) == Sol::FORBIDDEN)
		fout << "BM[1]"; // red
	else if (winner(treeInd) == Who::BLACK)
		fout << "TE[1]"; // green
	else if (winner(treeInd) == Who::WHITE)
		fout << "IT[]"; // blue
	else if (winner(treeInd) == Who::DRAW)
		error("TODO...");
	else if (winner(treeInd) == Who::NONE)
		; // unsolved node
	else
		error("???");

	fout << "\n";
}

inline void Tree::writeSGF0_link(ofstream &fout, Long_I treeInd_from, Int_I forkInd, const string &prefix, Trans_I trans) const
{
	Char BW, x, y; // letter B or letter W
	const Node & node_from = m_nodes[treeInd_from];
	Int i, nnext;
	Who who;
	Linkp plink = next(treeInd_from, forkInd);
	Long treeInd_to = plink->to();

	if (trans.flip())
		who = node_from.who();
	else
		who = ::next(node_from.who());

	if (who == Who::BLACK)
		BW = 'B';
	else if (who == Who::WHITE)
		BW = 'W';
	else
		error("illegal stone color");

	fout << ";" << BW;
	if (plink->ispass()) // pass
		fout << "[]";
	else {
		x = plink->x(); y = plink->y();
		transf(x, y, trans.rot());
		fout << '[' << char('a' + x) << char('a' + y) << "]";
	}

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

inline Bool Tree::nextMove_exist(Move mov, Long_I treeInd) const
{
	Int i, j;
	vector<Move> moves;
	const Node &node = m_nodes[treeInd];
	for (i = node.nnext() - 1; i >= 0; --i) {
		if (node.next(i)->move() == mov)
			return true;
	}
	return false;
}

inline Bool Tree::is_ko_node(Long_I treeInd) const
{
	return m_nodes[treeInd].is_ko_node();
}

inline Bool Tree::is_ko_child(Long_I treeInd, Int_I forkInd) const
{
	Linkp pLink = next(treeInd, forkInd);
	const Node & next_node = m_nodes[pLink->to()];
	return !pLink->isko() && next_node.is_ko_node();
}

inline MovRet Tree::prompt_move(Long_O child_treeInd, Long_I treeInd)
{
	Int i;
	MovRet ret;
	Int x, y;

	// display board
	cout << "current tree index : " << treeInd << endl;
	disp_board(treeInd);

	for (i = 0; i < 1000; ++i) {
		inp.num2(x, y, "input x y (negative to pass)");
		if (x < 0 || y < 0) {
			if (nextMove_exist(Move(Act::PASS), treeInd)) {
				if (inp.Bool("move exists, does all moves exist?")) {
					return MovRet::ALL_EXIST;
				}
				cout << "try again!" << endl;
				continue;
			}
			return pass(child_treeInd, treeInd);
		}
		else {
			// make a move
			BoardRef board = get_board(treeInd);
			// check existence
			if (nextMove_exist(Move(x, y), treeInd)) {
				if (inp.Bool("move exists, does all moves exist?")) {
					return MovRet::ALL_EXIST;
				}
				cout << "try again!" << endl;
				continue;
			}
			// check legal and number of removal
			if (check(x, y, treeInd) < 0)
				cout << "illegal move, try again!" << endl;
			// check dumb eye filling
			if (board.is_dumb_eye_filling(x, y, ::next(who(treeInd)))) {
				cout << "dumb eye filling, try again!" << endl;
			}
			if (board.is_dumb_2eye_filling(x, y, ::next(who(treeInd)))) {
				cout << "dumb 2-eye filling, try again!" << endl;
			}			
			
			ret = place(child_treeInd, x, y, treeInd);

			if (ret == MovRet::ILLEGAL) {
				cout << "illegal move, try again!" << endl;
				continue;
			}
			else if (ret == MovRet::KO_LN) {
				// ko link created
				cout << "ko, try again!" << endl;
				continue;
			}
			else {
				return ret;
			}
		}
	}
	error("should not reach here");
	return MovRet::ILLEGAL;
}

inline MovRet Tree::rand_smart_move(Long_O child_treeInd, Long_I treeInd)
{
	Bool exist, exist_pass = false;
	Int i, j, Nx = board_Nx(), Ny = board_Ny(), Nxy = Nx*Ny;
	MovRet ret;
	Char x0, y0, x, y;
	Who who = ::next(Tree::who(treeInd));
	VecInt xy;
	Node & node = m_nodes[treeInd];
	BoardRef board = get_board(treeInd);
	vector<Move> eat_pos;

	// consider placing that can eat stones
	board.eat_list(eat_pos, who);
	for (i = 0; i < eat_pos.size(); ++i) {
		x = eat_pos[i].x(); y = eat_pos[i].y();
		// check existence
		if (nextMove_exist(Move(x, y), treeInd))
			continue;
		// check legal and number of removal
		ret = place(child_treeInd, x, y, treeInd);
		if (ret == MovRet::ILLEGAL)
			continue; // illegal move, no change
		return ret;
	}

	// random sequence of all grid points on board
	randPerm(xy, Nxy);

	// search xy for a new legal move (not in Node::m_next)
	for (i = 0; i < Nxy; ++i) {
		x = xy[i] % Nx; y = xy[i] / Nx;
		// check existence
		if (nextMove_exist(Move(x, y), treeInd))
			continue;
		// check legal and number of removal
		if (check(x, y, treeInd) < 0)
			continue;
		// check dumb eye filling
		if (board.is_dumb_eye_filling(x, y, who)) {
			continue;
		}
		if (board.is_dumb_2eye_filling(x, y, who)) {
			continue;
		}

		ret = place(child_treeInd, x, y, treeInd);
		if (ret == MovRet::ILLEGAL)
			error("impossible!");
		return ret;
	}

	// no non-dumb placing left, consider passing
	if (!nextMove_exist(Move(Act::PASS), treeInd)) {
		return pass(child_treeInd, treeInd);
	}

	// all leagl moves already exist
	return MovRet::ALL_EXIST;
}

inline Int Tree::rand_game(Long_I treeInd0, Bool_I out)
{
	Int i;
	MovRet ret;
	Long treeInd = treeInd0, child_treeInd;

	// debug: edit board here
	/*tree.place(0, 0); tree.place(1, 2);
	tree.place(2, 2); tree.place(1, 1);
	tree.place(0, 2); tree.place(0, 1);
	tree.place(2, 1); tree.place(1, 0);
	tree.pass();*/
	// end edit board

	if (out) disp_board(treeInd);

	for (i = 1; i < 10000; ++i) {
		ret = rand_smart_move(child_treeInd, treeInd);
		if (ret == MovRet::ALL_EXIST) {
			if (out) cout << "all moves exists\n\n\n";
			break;
		}
		if (ret == MovRet::DB_PAS_END) {
			if (out) cout << "double pass !\n\n\n";
			break;
		}
		if (out) cout << "step " << i << endl;
		if (out) disp_board(treeInd);
		treeInd = child_treeInd;
	}

	if (out) cout << "game over!" << "\n\n";

	solve_end(treeInd);

	if (winner(treeInd) == Who::BLACK) {
		if (out) cout << "black wins!";
		if (out) cout << "  (score: " << 0.5*score2(treeInd) << ")\n\n";
	}
	else if (winner(treeInd) == Who::WHITE) {
		if (out) cout << "white wins!";
		if (out) cout << "  (score: " << 0.5*score2(treeInd) << ")\n\n";
	}
	else { // draw
		if (out) cout << "draw!\n\n";
	}
	if (out) writeSGF("randdom_game.sgf");

	return 0;
}

inline Int Tree::resolve_ko(Long_I treeInd_to)
{
	Int i, j, Nko = m_unreso_ko_links.size();
	Node &node = m_nodes[treeInd_to];
	Linkp plink;
	for (i = node.nlast() - 1; i >= 0; --i) {
		plink = node.last(i);
		if (plink->isko()) {
			plink->resolve();
			resolve_ko_record(plink->from(), treeInd_to);
			--Nko;
		}
	}
	return Nko;
}

inline void Tree::resolve_ko_record(Long_I treeInd_from, Long_I treeInd_to)
{
	for (Int i = m_unreso_ko_links.size() - 1; i >= 0; --i) {
		if (m_unreso_ko_links[i]->from() == treeInd_from
			&& m_unreso_ko_links[i]->to() == treeInd_to) {
			m_unreso_ko_links.erase(m_unreso_ko_links.begin() + i);
			return;
		}
	}
}

inline Int Tree::shift_ko_links(Long_I treeInd_from, Int_I forkInd)
{
	Int i;
	Int count = 0;
	Long treeInd = next(treeInd_from, forkInd)->to();
	Node & node = m_nodes[treeInd];
	vector<Linkp> plinks;
	Linkp plink, plink_save = unlink(treeInd_from, forkInd);

	// ko links from downstream to upstream converts to normal links
	ko_links(plinks, treeInd);
	for (i = plinks.size() - 1; i >= 0; --i) {
		if (islinked(plinks[i]->to(), treeInd) == 1 && plinks[i]->resolved()) {
			plinks[i]->ko_link_2_link();
			++count;
		}
	}
	// change all parents as descendents with a ko link
	if (count > 0) {
		for (i = node.nlast() - 1; i >= 0; --i) {
			plink = node.last(i);
			if (!plink->isko()) {
				plink->link_2_ko_link();
				m_unreso_ko_links.push_back(plink);
			}
		}
	}
	relink(plink_save);
	return count;
}

inline void Tree::solve_end(Long_I treeInd)
{
	// TODO check if this is an end node
	if (!isend(treeInd))
		error("Tree::solve_end(): unkown error!");

	calc_score(treeInd); calc_sol(treeInd);
}

Who Tree::winner(Long treeInd) const
{
	Sol sol = solution(treeInd);
	Who player = who(treeInd);
	return sol2winner(player, sol);
}

void Tree::calc_sol(Long_I treeInd)
{
	Sol sol = sco22sol(score2(treeInd), who(treeInd));
	set_solution(sol, treeInd);
}

void Tree::calc_ko_sol(Long_I treeInd)
{
	Sol sol = sco22sol(score2(treeInd), who(treeInd));
	set_solution(sol2ko_sol(sol), treeInd);
}

inline Int Tree::score2(Long_I treeInd) const
{
	Int sco2 = m_nodes[treeInd].score2();
	if (sco2 < 0 || sco2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	return sco2;
}

inline void Tree::set_score2(Int score2, Long_I treeInd)
{
	if (score2 < 0 || score2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	m_nodes[treeInd].set_sco2(score2);
}

inline Sol Tree::solution(Long_I treeInd) const
{
	return m_nodes[treeInd].solution();
}

inline void Tree::set_solution(Sol_I sol, Long_I treeInd)
{
	m_nodes[treeInd].set_solution(sol);
}

inline Bool Tree::solved(Long_I treeInd) const
{
	Sol sol = solution(treeInd);
	if (sol == Sol::GOOD || sol == Sol::BAD || sol == Sol::FAIR)
		return true;
	return false;
}

inline Sol & Tree::best_child_sol(Long_I treeInd) const
{
	Int i, sco2;
	Sol best_sol = Sol::BAD, sol;
	const Node & node = m_nodes[treeInd];
	for (i = 0; i < node.nnext(); ++i)
	{
		sol = solution(node.next(i)->to());
		if (best_sol - sol < 0) {
			best_sol = sol;
		}
	}
	return best_sol;
}

inline Int & Tree::best_child_sco2(Long_I treeInd) const
{
	Int i, sco2, max_sco2 = -10000;
	const Node & node = m_nodes[treeInd];
	for (i = 0; i < node.nnext(); ++i)
	{
		sco2 = score2(node.next(i)->to());
		if (max_sco2 < sco2) {
			max_sco2 = sco2;
		}
	}
	return max_sco2;
}

void Tree::calc_score(Long_I treeInd) // calculate territory and save to m_territory2
{
	Int sco2 = get_board(treeInd).calc_territory2(who(treeInd));
	set_score2(sco2, treeInd);
}

#include "tree_solve.inl"
