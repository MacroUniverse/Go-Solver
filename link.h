#pragma once
#include "move.h"

enum class LnType : Char
{
	// normal links
	SIMPLE, // simple link: link with no transformation
	TRANS,  // trans link: link with transformation
	// ko links
	KO_S,   // simple ko link: ko link with no transformation
	KO_T,	// trans ko link: ko link with transformation
	// markers
	INIT,   // init link: for 0-th node
	END     // end link: for end node
};

typedef const LnType &LnType_I;
typedef LnType &LnType_O, &LnType_IO;

class Link;
typedef const Link &Link_I;
typedef Link &Link_O, &Link_IO;

class Linkp;
typedef const Linkp Linkp_I;
typedef Linkp &Linkp_O, &Linkp_IO;

inline Bool istrans(LnType_I type)
{
	return type == LnType::TRANS || type == LnType::KO_T;
}

inline Bool isko(LnType_I type)
{
	return type == LnType::KO_S || type == LnType::KO_T;
}

// short link
class Link : public Move
{
protected:
	// basic members
	LnType m_type; // link type
	Long m_from; // tree index of source node
	Long m_to; // tree index of target node

	// other members
	Trans m_trans; // transformation
	Bool m_reso; // if ko links is resolved

public:

	Link(): m_reso(false) {}

	const LnType &type() const { return m_type; }
	const Long &from() const { return m_from; }
	const Long &to() const { return m_to; }
	const Move &move() const { return *this; }
	const Bool isinit() const { return m_type == LnType::INIT; }
	const Bool isend() const { return m_type == LnType::END; }
	const Bool isko() const { return ::isko(m_type); }
	const Bool istrans() const { return ::istrans(m_type); }
	
	void link(LnType_I type, Long_I from, Long_I to, Move_I move)
	{
		if (::istrans(type))
			error("not a simple link!");
		m_type = type;
		m_from = from;
		m_to = to;
		Move::operator=(move);
	}

	void link(LnType_I type, Long_I from, Long_I to, Move_I move, Trans_I trans)
	{
		if (!::istrans(type))
			error("not a trans link!");
		m_type = type;
		m_from = from;
		m_to = to;
		m_trans = trans;
		Move::operator=(move);
	}

	void init()
	{
		m_type = LnType::INIT;
		m_from = -1;
		m_to = 0;
		Move::init();
	}

	void end(Long_I treeInd)
	{
		m_type = LnType::INIT;
		m_from = treeInd;
		m_to = -1;
		Move::end();
	}

	const Trans &trans() const
	{
		if (!::istrans(m_type))
			error("not a trans link!");
		return m_trans;
	}
	
	Bool resolved() const
	{
		if (!::isko(m_type))
			error("not a ko link!");
		return m_reso;
	}
	
	void resolve()
	{
		if (!::isko(m_type))
			error("not a ko link!");
		m_reso = true;
	}

	// only support ko/non-ko conversion for now
	void convert(LnType_I type)
	{
		if (::isko(m_type)) {
			// from ko link
			if (!::isko(type)) {
				// to non-ko link
				if (m_type == LnType::KO_S) {
					m_type = LnType::SIMPLE;
				}
				else { // m_type == LnType::KO_T
					m_type = LnType::TRANS;
				}
			}
		}
		else {
			// from non-ko link
			if (::istrans(m_type)) {
				// to ko link
				if (m_type == LnType::SIMPLE) {
					m_type = LnType::KO_S;
				}
				else { // m_type == LnType::TRANS
					m_type = LnType::KO_T;
				}
			}
		}
		error("unsupported conversion");
	}

	void ko_link_2_link()
	{
		if (m_type == LnType::KO_S) {
			m_type = LnType::SIMPLE;
			m_reso = false;
		}
		else if (m_type == LnType::KO_T) {
			m_type = LnType::TRANS;
			m_reso = false;
		}
		else
			error("not a ko link");
	}

	void link_2_ko_link()
	{
		if (m_type == LnType::SIMPLE) {
			m_type = LnType::KO_S;
			m_reso = false;
		}
		else if (m_type == LnType::TRANS) {
			m_type = LnType::KO_T;
			m_reso = false;
		}
		else
			error("not a ko link");
	}
};

class Linkp
{
protected:
	// index for Link::m_links
	Long m_linkInd;

	// all links stored here
	static vector<Link> m_links;

public:

	Linkp() : m_linkInd(-1) {}
	Linkp(Long_I linkInd) : m_linkInd(linkInd) {}

	// create new un-initialized link and return pointer
	static Linkp newlink()
	{
		m_links.emplace_back();
		return Linkp(m_links.size() - 1);
	}

	Link* operator->() const
	{
		return &m_links[m_linkInd];
	}

	friend Bool operator==(Linkp_I lhs, Linkp_I rhs);
};



inline Bool operator==(Linkp_I lhs, Linkp_I rhs)
{
	return lhs.m_linkInd == rhs.m_linkInd;
}
