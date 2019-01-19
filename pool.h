#pragma once
#include "board.h"

// all situations in the tree, sorted for quick search
// sorting: each board is a radix 3 number, sort these numbers with ascending order
// a pool index (poolInd) is an index for m_boards, this index should never change for the same board
// m_treeInd will link to a tree node that is not a Act::PASS
// index to m_order is called (pool) order index (orderInd), this will change frequently for the same board!
class Pool
{
private:
	vector<Config> m_boards; // store all boards in the Pool
	// the corresponding node played by black/white
	// m_black_treeInd and m_white_treeInd should always be the same length and order of m_boards, use -1 if there is no link
	vector<Long> m_black_treeInd;
	vector<Long> m_white_treeInd;
	vector<Long> m_order; // m_board[m_order[i]] is in sorted order

public:
	Pool() {}

	Long size() const { return m_boards.size(); }

	// get a board reference by index
	const Config & operator[](Long_I ind) const { return m_boards[m_order[ind]]; }

	// get pool index from order index
	Long poolInd(Long_I orderInd) const;

	// search Pool: find poolInd so that m_board[poolInd] and raw_board have the same configuration
	// return 0, output ind: if s is found
	// return -1: if s is not found and board is too small
	// return  1: if s is not found and board is too large
	// return -2: if s is not found and board is in the middle, output ind so that m_pool[ind] < s < m_pool[ind+1]
	// return -3: if m_pool.size() < 1
	// TODO: improve implementation (stone by stone algorithm)
	// output flip and rotation calculated by Config::calc_trans()
	Int search(Config_O &board, Int_O &rotation, Bool_O &flip, Long_O &poolInd, Long_O &orderInd, RawBoard_I &raw_board);

	// add a new board (configuration) to the Pool
	// the board must already be transformed
	// orderInd is output by search() and search_ret is returned by search()
	// orderInd should be -3, -2, -1 or 1
	// flip and rotation are calculated by Config::calc_rot_flip()
	void push(Config_I &board, Int_I search_ret, Long_I orderInd, Who_I who, Long_I treeInd);

	// add a new situation to an existing configuration
	void link(Long_I orderInd, Who_I who, Long_I treeInd);

	// return the treeInd of a situation
	Long treeInd(Long_I poolInd, Who_I who) const;
};

Long Pool::poolInd(Long_I orderInd) const
{
	return m_order[orderInd];
}

Int Pool::search(Config_O &board, Trans_O &trans, Long_O &poolInd, Long_O &orderInd, RawBoard_I &raw_board)
{
	trans = raw_board.calc_trans();
	Config board;
	raw_board.transform(board, rotation, flip);
	Int ret = lookupInt(orderInd, *this, board);
	if (ret == 0)
		poolInd = m_order[orderInd];
	return ret;
}

inline void Pool::push(Config_I &board, Int_I search_ret, Long_I orderInd, Who_I who, Long_I treeInd)
{
	// transform the board first!
	m_boards.push_back(board);

	if (who == Who::BLACK) {
		m_black_treeInd.push_back(treeInd);
		m_white_treeInd.push_back(-1);
	}
	else if (who == Who::WHITE) {
		m_black_treeInd.push_back(-1);
		m_white_treeInd.push_back(treeInd);
	}

	Long poolInd = m_boards.size() - 1;
	if (search_ret == -2)
		m_order.insert(m_order.begin() + orderInd + 1, poolInd);
	else if (search_ret == -1)
		m_order.insert(m_order.begin(), poolInd);
	else if (search_ret == 1 || search_ret == -3)
		m_order.push_back(poolInd);
	else
		error("Pool::push(): unknown search_ret!");
}

inline void Pool::link(Long_I orderInd, Who_I who, Long_I treeInd)
{
	Long poolInd = this->poolInd(orderInd);

	if (who == Who::BLACK) {
		if (m_black_treeInd[poolInd] > -1)
			error("situation already exists!");
		m_black_treeInd[poolInd] = treeInd;
	}
	else if (who == Who::WHITE) {
		if (m_white_treeInd[poolInd] > -1)
			error("situation already exists!");
		m_white_treeInd[poolInd] = treeInd;
	}
	else
		error("illegal who!");
}

Long Pool::treeInd(Long_I poolInd, Who_I who) const
{
	if (who == Who::BLACK) {
		return m_black_treeInd[poolInd];
	}
	else if (who == Who::WHITE) {
		return m_white_treeInd[poolInd];
	}
	else
		error("illegal who!");
}
