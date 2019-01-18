#pragma once
#include "board.h"

// all situations in the tree, sorted for quick search
// sorting: each board is a radix 3 number, sort these numbers with ascending order
// a pool index (poolInd) is an index for m_boards, this index should never change for the same board
// m_treeInd will link to a tree node that is not a Act::PASS
// index to m_order is called (pool) order index (orderInd), this will change frequently for the same board!
class Pool
{
public:
	vector<Board> m_boards; // store all boards in the Pool
	vector<Long> m_black_treeInd; // the corresponding node played by black
	vector<Long> m_white_treeInd; // the corresponding node played by white
	vector<Long> m_order; // m_board[m_order[i]] is in sorted order

	Pool() {}

	Long size() const { return m_boards.size(); }

	// get a board by index
	const Board & operator[](Long_I ind) const { return m_boards[m_order[ind]]; }

	// search Pool: find poolInd so that m_board[poolInd] and raw_board have the same configuration
	// TODO: improve implementation (stone by stone algorithm)
	Int search(Long_O &poolInd, Long_O &orderInd, Board_I &raw_board);

	// add a new board to the Pool
	inline void push(Board_I &board, Int_I search_ret, Long_I orderInd, Long_I treeInd);
};

Int Pool::search(Long_O &poolInd, Long_O &orderInd, Board_I &raw_board)
{
	Int ret = lookupInt(orderInd, *this, raw_board);
	if (ret == 0)
		poolInd = m_order[orderInd];
	return ret;
}

// orderInd is output by search() and search_ret is returned by search()
// orderInd should be -3, -2, -1 or 1
inline void Pool::push(Board_I &board, Int_I search_ret, Long_I orderInd, Long_I treeInd)
{
	m_boards.push_back(board);
	m_treeInd.push_back(treeInd);
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
