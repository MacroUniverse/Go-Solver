#pragma once
#include "node.h"
#include "pool.h"
#include "boardref.h"

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
	vector<Long> m_ko_treeInds; // keep track of all the destinations of Ko links
	Sol m_ko_best; // best descendent solution except ko

	// === const functions ===

	// return a reference to the transformed Board of a node
	// rotation and flip are transformations needed to transform the board back
	BoardRef get_board(Long_I treeInd = -1) const;

	Long index() const { return m_treeInd; } // return current node index

	Long nnode() const { return m_nodes.size(); } // return number of nodes in the tree

	inline Bool isend(Long_I treeInd = -1) const; // if a node is an end node

	inline Bool is_next_ko(Long_I treeInd = -1, Int_I forkInd = 0) const;

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
		return m_nodes[last(treeInd, forkInd)];
	}

	// return an index of the next node of a node, fork can be negative
	inline Long next(Long_I treeInd = -1, Int_I forkInd = 0) const;

	const Node & nextNode(Long_I treeInd = -1, Int_I forkInd = 0) const // return a reference of the next node of a node
	{
		return m_nodes[next(treeInd, forkInd)];
	} // forkInd can be negative

	const Move & nextMove(Long_I treeInd = -1, Int_I forkInd = 0) const // return next move of a node
	{
		const Node & next_node = nextNode(treeInd, forkInd);
		return next_node.move(next_node.parent(treeInd));
	}

	Who who(Long_I treeInd = -1) const // who played the node
	{
		return m_nodes[def(treeInd)].who();
	}

	inline void disp_board(Long_I treeInd = -1) const; // display board

	// return 0 if new node created
	// return -1 if double pass caused game end
	// return -2 if double pass attempted but game did not finish (nothing done)
	// return 1 if linked to an another existing node
	inline Int pass(Long_I treeInd = -1);

	inline Int check(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int place(Char_I x, Char_I y, Long_I treeInd = -1);

	inline Int islinked(Long_I treeInd1, Long_I treeInd2); // check if node treeInd1 can lead to node treeInd2

														   // check if same board already exists in the Pool and decide if it is a Ko
	inline Int check_ko(Int_O &search_ret, Long_O &treeInd_match, Long_O &orderInd, Long_I treeInd, Config_I &config);

	inline void branch(vector<Long> &br, Long_I treeInd = -1); // get an index vector of the a branch ending at a node

	inline void writeSGF_old(const string &name); // write a branch to file

	inline void writeSGF(const string &name); // write the whole tree to file

	inline Long writeSGF0(ofstream &fout, Long_I treeInd = -1); // internal function called by writeSGF();

	inline void writeSGF01(ofstream &fout, Long_I treeInd, const string &prefix = ""); // internal function called by writeSGF0();

	inline Bool next_exist(Move mov, Long_I treeInd = -1); // does next node have this mov already?

														   // make a random move for a node
														   // only pass if all other moves are onsidered
	inline Int rand_move(Long_I treeInd = -1);

	// smarter random move for a node
	// will not do a dumb move (dumb eye filling or dumb big eye filling)
	// return 0 if successful and new node created
	// return 1 if linked to existing node and no Ko
	// return 2 if linked to existing node and has Ko
	// return 3 if sucessful but is a dumb move
	// return -1 if there is all legal moves already exists
	// return -2 if passed and game ends
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
	// return -1 if at least one downstream ko exists, and all other descendents have been solved
	//           all downstream ko stored in Tree::m_ko_treeInds, best downstream solution stored in Tree::m_ko_best
	inline Int solve(Long_I treeInd = -1);

	// the winner of a node
	Who winner(Long treeInd = -1) const;

	// calculate and update score for a node
	void calc_score(Long_I treeInd = -1);

	// update solution for a node based on scores and komi
	void calc_sol(Long_I treeInd = -1);

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
		return node(def(treeInd)).m_best_child_sol;
	}

	Int & best_child_sco2(Long_I treeInd = -1)
	{
		return node(def(treeInd)).m_best_child_sco2;
	}

	Bool & all_child_exist(Long_I treeInd = -1)
	{
		return m_nodes[def(treeInd)].m_all_child_exist;
	}

	// set default argument treeInd
	Long def(Long_I treeInd) const
	{
		return treeInd < 0 ? m_treeInd : treeInd;
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
	Config config; config.init();
	m_pool.push(config, -3, 0, Who::NONE, 0);
	m_nodes.push_back(Node()); m_nodes.back().init();
}

inline Bool Tree::isend(Long_I treeInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].isend())
		return true;
	return false;
}

inline Bool Tree::is_next_ko(Long_I treeInd /*optional*/, Int_I forkInd /*optional*/) const
{
	if (m_nodes[def(treeInd)].is_next_ko(forkInd))
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
	return  m_nodes[def(treeInd)].next(forkInd);
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
	const Long treeInd1 = def(treeInd);
	const BoardRef board = get_board(treeInd1);

	// check double pass
	for (i = 0; i < m_nodes[treeInd1].nlast(); ++i) {
		if (m_nodes[treeInd1].move(i).ispass()) {
			// double passed!
			if (m_nodes[treeInd1].nlast() > 1)
				error("unhandled case!");
			if (!board.is_game_end())
				return -2;
			// two situations have the same scores and solutions!
			calc_score(treeInd1);
			calc_sol(treeInd1);
			set_score2(inv_score2(score2(treeInd1)), last(treeInd1, i));
			set_solution(inv_sol(solution(treeInd1)), last(treeInd1, i));
			return -1;
		}
	}

	// check ko
	Int search_ret;
	Long orderInd, treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd1, board.config());

	if (ret == -1) { // situation exists, not a ko
		m_nodes[treeInd1].push_next(treeInd_found);
		if (treeInd < 0) m_treeInd = treeInd_found;
		return 1;
	}
	else if (ret == -2) { // situation exists, is a ko
		error("unlikely case, check this!");
	}
	else if (ret == 0) { // configuration does not exist
		error("impossible case, configuration must exist!");
	}
	else if (ret != -3)
		error("unknown return!");

	// ret == -3 now, configuration exists, new situation

	m_nodes.push_back(Node());
	m_nodes.back().pass(::next(who(treeInd1)), treeInd1, m_pool.poolInd(orderInd), board.trans());
	m_nodes[treeInd1].push_next(nnode() - 1);
	m_pool.link(orderInd, ::next(who(treeInd1)), nnode() - 1);

	if (treeInd < 0) m_treeInd = nnode() - 1;
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

// return 0 if new node created (m_treeInd++)
// return 1 if linked to an existing node (m_treeInd jumped)
// return -1 if illegal (nothing changes)
// return -2 if a ko is found and linked (m_treeInd unchanged)
// already has bound checking
inline Int Tree::place(Char_I x, Char_I y, Long_I treeInd /*optional*/)
{
	Int ret;
	const Long treeInd1 = def(treeInd);
	Board board;
	board = get_board(treeInd1);
	// first move
	if (treeInd1 == 0) {
		board.place(x, y, Who::BLACK);
		m_pool.push(board.config(), 1, -1, Who::BLACK, 1);
		m_nodes.push_back(Node());
		m_nodes.back().place(x, y, Who::BLACK, treeInd1, m_pool.size() - 1, board.trans());
		m_nodes[treeInd1].push_next(nnode() - 1);
		if (treeInd < 0) m_treeInd = 1;
		return 0;
	}

	// update board and check illegal move (Ko no checked!)
	if (board.place(x, y, ::next(who(treeInd1))))
		return -1;

	// check Ko
	Int search_ret;
	Long orderInd;
	Long treeInd_found;
	ret = check_ko(search_ret, treeInd_found, orderInd, treeInd1, board.config());
	if (ret < 0) { // board already exists
		if (ret == -1) { // not a ko
			m_nodes[treeInd1].push_next(treeInd_found);
			if (treeInd < 0) m_treeInd = treeInd_found;
			return 1;
		}
		else if (ret == -2) { // is a ko
			m_nodes[treeInd1].push_next(-treeInd_found - 1);
			return -2;
		}
		// ret == -3: not the same player, continue
	}

	// new situation
	m_nodes.push_back(Node()); // add Node to tree

	if (ret == -3) { // configuration exists
		m_pool.link(orderInd, ::next(who(treeInd1)), nnode() - 1);
		m_nodes.back().place(x, y, ::next(who(treeInd1)), treeInd1, m_nodes[treeInd_found].poolInd(), board.trans());
	}
	else { // configuration does not exist
		m_pool.push(board.config(), search_ret, orderInd, ::next(who(treeInd1)), nnode() - 1);
		m_nodes.back().place(x, y, ::next(who(treeInd1)), treeInd1, m_pool.size() - 1, board.trans());
	}

	m_nodes[treeInd1].push_next(nnode() - 1);
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
		if (treeInd == treeInd1) return 1; // found treeInd2
		if (treeInd == 0) return 0; // reached top of tree
		treeInd = m_nodes[treeInd].last(0); // single line, go up
	}
	for (i = 0; i < m_nodes[treeInd].nlast(); ++i) {
		if (islinked(treeInd1, last(treeInd, i)) == 1)
			return 1;
	}
	error("should not reach here!"); return 0;
}

// 'nodes[treeInd]' will produce 'config' in the next move
// if board already exists, output tree index of the same board
// return 0 if this is a new configuration
// return -1 if situation exists but not a ko
// return -2 if situation exists and is a ko
// return -3 if this is a new situation of an existing configuration
inline Int Tree::check_ko(Int_O &search_ret, Long_O &treeInd_match, Long_O &orderInd,
	Long_I treeInd, Config_I &config)
{
	Long poolInd;
	search_ret = m_pool.search(poolInd, orderInd, config);
	// config already exists
	if (search_ret == 0) {
		treeInd_match = m_pool.treeInd(poolInd, ::next(who(treeInd)));
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
	Long max_treeInd = writeSGF0(fout, 1);
	if (max_treeInd != nnode() - 1)
		error("writeSGF() nodes number does not match!");

	fout << ")\n";
	fout.close();
}

// return max node index written
// nodes should be written in the order of treeIndex
// recursively write the tree
inline Long Tree::writeSGF0(ofstream &fout, Long_I treeInd)
{
	Int i, nnext;
	Long treeInd1 = def(treeInd), treeInd2;
	static Long max_treeInd = treeInd1 - 1;
	if (treeInd == 1) max_treeInd = 0;

	// write one node
	writeSGF01(fout, treeInd1);
	++max_treeInd;

	// go down the branch if no fork
	for (i = 0; i < 10000; ++i) {
		nnext = m_nodes[treeInd1].nnext();
		if (nnext == 1) {
			if (isend(treeInd1)) {
				return max_treeInd; // game ends
			}
			treeInd2 = next(treeInd1);
			if (is_next_ko(treeInd1)) {
				writeSGF01(fout, treeInd2, "ko>");
				return max_treeInd; // ko link
			}
			if (treeInd2 < treeInd1) { // reached a link to an existing node
				writeSGF01(fout, treeInd2, ">"); // write a node to represent a link
				return max_treeInd;
			}
			if (treeInd2 != treeInd1 + 1)
				error("unknown!");
			treeInd1 = treeInd2;
			writeSGF01(fout, treeInd1);
			++max_treeInd;
		}
		else if (nnext == 0)
			return max_treeInd;
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
		if (is_next_ko(treeInd1, i)) {
			writeSGF01(fout, treeInd2, "ko>"); // write a node to represent a link
			continue;
		}
		if (treeInd2 < max_treeInd) { // reached a link to an existing node
			writeSGF01(fout, treeInd2, ">"); // write a node to represent a link
			continue;
		}
		if (treeInd2 != max_treeInd + 1)
			error("unknown!");

		writeSGF0(fout, treeInd2);
		fout << ')';
	}
	return max_treeInd;
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
	if (solved(treeInd1))
		fout << 0.5*score2(treeInd1);
	// end title
	fout << "]";

	// add green (black wins) or blue (white wins) mark
	if (winner(treeInd1) == Who::BLACK)
		fout << "TE[1]"; // green
	else if (winner(treeInd1) == Who::WHITE)
		fout << "IT[]"; // blue
	else if (winner(treeInd1) == Who::DRAW)
		error("TODO...");
	else if (winner(treeInd1) == Who::NONE)
		;
	else
		error("???");

	fout << "\n";
}

inline Bool Tree::next_exist(Move mov, Long_I treeInd /*optional*/)
{
	Int i;
	Long treeInd1 = def(treeInd);
	Node &node = m_nodes[treeInd1];
	for (i = 0; i < node.nnext(); ++i) {
		if (nextNode(treeInd1, i).move() == mov)
			return true;
	}
	return false;
}

// return 0 if successful and new node created
// return 1 if linked to existing node and no Ko
// return 2 if linked to existing node and has Ko
// return -1 if there is all available moves already exists
// return -2 if double pass caused end of game
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
		if (get_board(treeInd1).is_dumb_eye_filling(x, y, ::next(who(treeInd1)))) {
			continue;
		}
		if (get_board(treeInd1).is_dumb_2eye_filling(x, y, ::next(who(treeInd1)))) {
			continue;
		}
		ret = place(x, y, treeInd1);
		if (ret == -1)
			continue; // illegal move, no change
		else if (ret == 0)
			return 0; // new node created
		else if (ret == 1)
			return 1;  // linked to an existing node, no Ko
		else if (ret == -2)
			return 2; // linked to existing node, has Ko
	}

	// no non-dumb placing left, consider passing
	if (!next_exist(Move(Act::PASS), treeInd1)) {
		ret = pass(treeInd1);
		if (ret == -1)
			return -2; // double pass successful
		else if (ret == 0)
			return 0; // first pass
		else if (ret == -2) {
			return -1; // double pass not allowed
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
	Int i, ret, child_treeInd;
	Long treeInd1 = def(treeInd);
	Bool found_ko = false;
	Int child_sco2;
	Sol child_sol;

	// if already solved
	if (solution(treeInd1) != Sol::UNKNOWN) {
		if (solution(treeInd1) == Sol::KO_ONLY) {
			error("unhandled situation!");
		}
		return 0;
	}

	// enumerate children
	Int nskip = 35; // debug
	for (i = 0; i < 1000; ++i) {
		ret = rand_smart_move(treeInd1);
		if (nnode() == 100) { // debug
			writeSGF("test.sgf");
		}
		// end debug
		if (ret == 0 || ret == 1 || ret == 3) {
			// new node created (might be a first pass)
			if (ret == 0 || ret == 3) {
				child_treeInd = nnode() - 1;
				// debug, display board
				cout << "largest treeInd = " << nnode() - 1 << endl;
				disp_board(nnode() - 1); cout << "\n\n" << endl;
				cout << ""; // debug break point
							// end debug
			}
			// linked to existing node, no Ko
			else if (ret == 1) {
				child_treeInd = next(treeInd1, -1);
			}
			else
				error("unhandled case!");

			ret = solve(child_treeInd);
			if (ret == -1) {
				continue; // downstream ko exists
			}
			else if (ret == 1) {
				// double pass solved two nodes
				return 0;
			}
			else if (ret != 0) {
				error("unhandled case!");
			}
			child_sco2 = score2(child_treeInd);
			child_sol = solution(child_treeInd);

			if (best_child_sco2(treeInd1) < child_sco2) {
				best_child_sco2(treeInd1) = child_sco2;
				best_child_sol(treeInd1) = child_sol;
				if (best_child_sol(treeInd1) == Sol::GOOD) {
					set_solution(Sol::BAD, treeInd1);
					set_score2(inv_score2(best_child_sco2(treeInd1)), treeInd1);
					return 0; // debug break point
				}
			}
		}
		// linked to existing node, has Ko
		else if (ret == 2) {
			Long child_treeInd = next(treeInd1, -1);
			m_ko_treeInds.push_back(child_treeInd);
			found_ko = true;
			continue;
		}
		// all children exist
		else if (ret == -1) {
			if (node(34).nnext() == 0) {
				// double pass was the only choice but not allowed...
				set_solution(Sol::FORBIDDEN, treeInd1);
				set_score2(inv_score2(best_child_sco2(treeInd1)), treeInd1);
			}
			all_child_exist(treeInd1) = true;
			// all children solved
			if (!found_ko) {
				if (best_child_sol(treeInd1) == Sol::FAIR) {
					set_solution(Sol::FAIR, treeInd1);
					set_score2(inv_score2(best_child_sco2(treeInd1)), treeInd1);
				}
				else { // best == Sol::BAD
					set_solution(Sol::GOOD, treeInd1);
					set_score2(inv_score2(best_child_sco2(treeInd1)), treeInd1);
				}
				return 0; // debug break point
			}
			// not all children solved
			else {
				set_solution(Sol::KO_ONLY, treeInd1);
				set_score2(inv_score2(best_child_sco2(treeInd1)), treeInd1);
				return -1; // debug break point
			}
		}
		// new pass node created, double pass caused end of game
		else if (ret == -2) {
			return 1;
		}
		else
			error("unhandled return!");
	}
	error("unkown error!"); return -1;
}

Who Tree::winner(Long treeInd) const
{
	Long treeInd1 = def(treeInd);
	Sol sol = solution(treeInd1);
	Who player = who(treeInd1);

	if (sol == Sol::UNKNOWN)
		return Who::NONE;
	if (player == Who::BLACK) {
		if (sol == Sol::GOOD)
			return Who::BLACK;
		if (sol == Sol::BAD)
			return Who::WHITE;
		if (sol == Sol::FAIR)
			return Who::DRAW;
	}
	else if (player == Who::WHITE) {
		if (sol == Sol::GOOD)
			return Who::WHITE;
		if (sol == Sol::BAD)
			return Who::BLACK;
		if (sol == Sol::FAIR)
			return Who::DRAW;
	}
	else
		error("illegal Who input");
}

void Tree::calc_sol(Long_I treeInd)
{
	Long treeInd1 = def(treeInd);

	Int score4_draw = board_Nx()*board_Ny() * 2;
	if (who(treeInd1) == Who::BLACK)
		score4_draw += komi2();
	else if (who(treeInd1) == Who::WHITE)
		score4_draw -= komi2();
	else
		error("illegal player!");
	Int score4 = 2 * score2(treeInd1);
	if (score4 > score4_draw)
		set_solution(Sol::GOOD, treeInd1);
	else if (score4 < score4_draw)
		set_solution(Sol::BAD, treeInd1);
	else
		set_solution(Sol::FAIR, treeInd1);
}

inline Int Tree::score2(Long_I treeInd) const
{
	Int sco2 = node(def(treeInd)).m_score2;
	if (sco2 < 0 || sco2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	return sco2;
}

inline void Tree::set_score2(Int score2, Long_I treeInd)
{
	if (score2 < 0 || score2 > 2 * board_Nx() * board_Ny())
		error("illegal score");
	node(def(treeInd)).m_score2 = score2;
}

inline Sol Tree::solution(Long_I treeInd) const
{
	return node(def(treeInd)).m_sol;
}

inline void Tree::set_solution(Sol_I sol, Long_I treeInd)
{
	node(def(treeInd)).m_sol = sol;
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
	node(treeInd1).m_score2 = get_board(def(treeInd1)).calc_territory2(who(treeInd1));
}
