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

typedef Link *Linkp;
typedef const Link *const Linkp_I;

class Slink;
typedef const Slink &Slink_I;
typedef Slink &Slink_O, &Slink_IO;

typedef Slink *Slinkp;
typedef const Slink *const Slinkp_I;

class Tlink;
typedef const Tlink &Tlink_I;
typedef Tlink &Tlink_O, &Tlink_IO;

typedef Tlink *Tlinkp;
typedef const Tlink *const Tlinkp_I;

class SKlink;
typedef const SKlink &SKlink_I;
typedef SKlink &SKlink_O, &SKlink_IO;

typedef SKlink *SKlinkp;
typedef const SKlink *const SKlinkp_I;

class TKlink;
typedef const TKlink &TKlink_I;
typedef TKlink &TKlink_O, &TKlink_IO;

typedef TKlink *TKlinkp;
typedef const TKlink *const TKlinkp_I;

// short link
class Link : public Move
{
protected:
	LnType m_type; // link type
	Long m_from; // tree index of source node
	Long m_to; // tree index of target node

public:
	Link() {}
	void link(LnType_I type, Long_I from, Long_I to, Move_I move)
	{
		m_type = type;
		m_from = from;
		m_to = to;
		Move::operator=(move);
	}
	Linkp_I const ptr() const { return this; }
	const LnType &type() const { return m_type; }
	const Long &from() const { return m_from; }
	const Long &to() const { return m_to; }
	const Move &move() const { return *this; }
	const Bool isinit() const { return m_type == LnType::INIT; }
	const Bool isend() const { return m_type == LnType::END; }
	const Bool isko() const { return m_type == LnType::KO_S || m_type == LnType::KO_T; }
	const Bool istrans() const { return m_type == LnType::TRANS || m_type == LnType::KO_T; }
	virtual const Bool resolved() const
	{
		error("not a ko link!");
	}
	
	virtual void resolve()
	{
		error("not a ko link!");
	}
};

// simple link
class Slink : public Link
{
protected:
	static vector<Slink> m_links;

public:
	Slink() {}

	void convert(SKlinkp_I sklink)
	{
		link(sklink->from(), sklink->to(), sklink->move());
	}

	void init()
	{
		Link::link(LnType::INIT, -1, 0, Move(Act::INIT));
	}

	void end(Long_I treeInd)
	{
		Link::link(LnType::END, treeInd, -1, Move(Act::END));
	}

	void link(Long_I from, Long_I to, Move_I move)
	{
		Link::link(LnType::SIMPLE, from, to, move);
	}

	friend Linkp ko_link_2_link(Linkp_I pLink);
	friend Linkp link_2_ko_link(Linkp_I pLink);
	friend class Tree;
};

// long link
class Tlink : public Link
{
protected:
	Trans m_trans; // transformation from source to target
	static vector<Tlink> m_links;

public:
	Tlink() {}

	void convert(TKlinkp_I pTKlink)
	{
		link(pTKlink->from(), pTKlink->to(), pTKlink->move(), pTKlink->trans());
	}

	const Trans &trans() const
	{
		return m_trans;
	}

	void link(Long_I from, Long_I to, Move_I move, Trans_I trans)
	{
		Link::link(LnType::TRANS, from, to, move);
		m_trans = trans;
	}

	void link(LnType_I type, Long_I from, Long_I to, Move_I move, Trans_I trans)
	{
		Link::link(type, from, to, move);
		m_trans = trans;
	}

	friend Linkp ko_link_2_link(Linkp_I pLink);
	friend Linkp link_2_ko_link(Linkp_I pLink);
	friend class Tree;
};

// simple ko link
class SKlink : public Link
{
private:
	Bool m_reso; // if the ko link has been resolved
	static vector<SKlink> m_links;

public:
	SKlink(): m_reso(false) {}

	// convert from a trans link
	void convert(Slinkp_I pSlink)
	{
		link(pSlink->from(), pSlink->to(), pSlink->move());
	}

	Bool resolved() const
	{
		return m_reso;
	}

	void resolve()
	{
		m_reso = true;
	}

	void link(Long_I from, Long_I to, Move_I move)
	{
		Link::link(LnType::KO_S, from, to, move);
	}

	friend Linkp ko_link_2_link(Linkp_I pLink);
	friend Linkp link_2_ko_link(Linkp_I pLink);
	friend class Tree;
};


// trans ko link
class TKlink : public Tlink
{
private:
	Bool m_reso; // if the ko link has been resolved
	static vector<TKlink> m_links;

public:
	TKlink(): m_reso(false) {}
	
	// convert from long link
	void convert(Tlinkp_I pTlink)
	{
		link(pTlink->from(), pTlink->to(), pTlink->move(), pTlink->trans());
	}

	Bool resolved() const
	{
		return m_reso;
	}

	void resolve()
	{
		m_reso = true;
	}

	void link(Long_I from, Long_I to, Move_I move, Trans_I trans)
	{
		Tlink::link(LnType::KO_T, from, to, move, trans);
	}

	friend Linkp ko_link_2_link(Linkp_I pLink);
	friend Linkp link_2_ko_link(Linkp_I pLink);
	friend class Tree;
};

inline Linkp ko_link_2_link(Linkp_I pLink)
{
	if (pLink->type() == LnType::KO_S) {
		Slink::m_links.emplace_back();
		Slink::m_links.back().convert((SKlinkp)pLink);
		return Slink::m_links.back().ptr();
	}
	else if (pLink->type() == LnType::KO_T) {
		Tlink::m_links.emplace_back();
		Tlink::m_links.back().convert((TKlinkp)pLink);
		return Tlink::m_links.back().ptr();
	}
	else
		error("not a ko link");
}

inline Linkp link_2_ko_link(Linkp_I pLink)
{
	if (pLink->type() == LnType::SIMPLE) {
		SKlink::m_links.emplace_back();
		SKlink::m_links.back().convert((Slinkp)pLink);
		return Slink::m_links.back().ptr();
	}
	else if (pLink->type() == LnType::TRANS) {
		TKlink::m_links.emplace_back();
		TKlink::m_links.back().convert((Tlinkp)pLink);
		return TKlink::m_links.back().ptr();
	}
	else
		error("not a normal link");
}
