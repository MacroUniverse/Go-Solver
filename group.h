#pragma once
#include "move.h"

// a group of connected stones
class Group
{
private:
	vector<Move> m_pos;
	vector<Move> m_qi;
public:
	// === constant functions ===
	const vector<Move> & qi() const
	{
		return m_qi;
	}

	vector<Move> & qi()
	{
		return m_qi;
	}

	vector<Move> & pos()
	{
		return m_pos;
	}
};
