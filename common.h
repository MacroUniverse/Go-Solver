#pragma once
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#ifndef NDEBUG
#define _RAND_SEED_ 0
#define GOS_CHECK_BOUND
#endif

#include "SLISC/slisc.h"
#include "SLISC/random.h"
#include "SLISC/search.h"
#include "SLISC/time.h"

using namespace slisc;
using std::vector; using std::string;
using std::ofstream; using std::cout;
using std::cin; using std::endl;

enum class Who : Char { NONE, WHITE, BLACK, DEFAULT, DRAW }; // TODO: make this a char type
typedef const Who Who_I;
typedef Who Who_IO, Who_O;

enum class Act : Char { PLACE, PASS, EDIT, INIT }; // TODO: make this a char type
typedef const Act Act_I;

enum class Sol : Char { BAD, FAIR, GOOD, UNKNOWN, KO_ONLY };
typedef const Sol Sol_I;

// convert who to integer
inline Int who2int(Who_I who)
{
	if (who == Who::NONE)
		return 0;
	if (who == Who::BLACK)
		return 2;
	if (who == Who::WHITE)
		return 1;
	error("illegal who!");
}

// inverse color
inline Who next(Who_I s)
{
	if (s == Who::BLACK) return Who::WHITE;
	if (s == Who::WHITE) return Who::BLACK;
	if (s == Who::NONE) return Who::BLACK; // empty board
	error("opposite(Who_I): unknown error!");
}

// board size Nx, may only set once
inline Char board_Nx(Char_I Nx = -1)
{
	static Char Nx0 = -1;
	if (Nx0 < 0) {
		if (Nx > 0)
			Nx0 = Nx;
		else
			error("board_Nx(): Nx0 not uninitialized, or non-positive input!");
	}
	return Nx0;
}

// board size Ny, may only set once
inline Char board_Ny(Char_I Ny = -1)
{
	static Char Ny0 = -1;
	if (Ny0 < 0) {
		if (Ny > 0)
			Ny0 = Ny;
		else
			error("board_Ny(): size must be positive!");
	}
	return Ny0;
}

// board size Ny, may only set once
// should only be integer or half integers
inline Int komi2(Int_I k2 = -1132019)
{
	static Doub k20 = -1132019;
	if (k2 != -1132019) {
		if (abs(k2) > 2 * board_Nx() * board_Ny())
			error("komi too large or too small! One side always win!");
		k20 = k2; // set komi
	}
	else if (k20 == -1132019)
		error("komi(): komi unset!");
	return k20;
}

// complementary territory (opponent's territory)
inline Int inv_territory2(Int_I territory2)
{
	return 2 * board_Nx()*board_Ny() - territory2;
}

// complementary score (opponent's territory)
inline Int comp_score2(Int_I score2)
{
	return inv_territory2(score2);
}

// complementary solution (opponent's solution)
inline Sol inv_sol(Sol_I sol)
{
	if (sol == Sol::GOOD)
		return Sol::BAD;
	if (sol == Sol::BAD)
		return Sol::GOOD;
	// KO_ONLY or UNKNOWN
	return sol;
}

// board transformation
class Trans
{
private:
	Int m_rot;
	Bool m_flip;
public:
	Trans(): m_rot(0), m_flip(false) {};
	Trans(Int_I rot, Bool_I flip): m_rot(rot), m_flip(flip) {}
	Int rot() const { return m_rot; }
	Bool flip() const { return m_flip; }
	void set_rot(Int_I rot) { m_rot = rot; }
	void set_flip(Bool_I flip) { m_flip = flip; }
};

typedef Trans Trans_O, Trans_IO;
typedef const Trans Trans_I;

// get inverse transform
inline Trans inv(Trans_I trans)
{
	Trans out = trans;
	if (out.rot() != 0)
		out.set_rot(4 - trans.rot());
	return out;
}

// transform coordinates
inline void transf(Char_IO &x, Char_IO &y, Who_O &who, Trans trans)
{
	Char temp;
	if (trans.rot() == 1) {
		temp = x; x = y; y = -temp;
	}
	else if (trans.rot() == 2) {
		x = -x; y = -y;
	}
	else if (trans.rot() == 3) {
		temp = x; x = -y; y = temp;
	}
	if (trans.flip)
		who = next(who);
}

// inverse transform coordinates
inline void inv_transf(Char_IO &x, Char_IO &y, Who_O &who, Trans trans)
{
	Char temp;
	if (trans.rot() == 1) {
		temp = x; x = -y; y = temp;
	}
	else if (trans.rot() == 2) {
		x = -x; y = -y;
	}
	else if (trans.rot() == 3) {
		temp = x; x = y; y = -temp;
	}
	if (trans.flip)
		who = next(who);
}

// calculate trans1 += trans2
inline void operator+=(Trans_IO &trans1, Trans_I trans2)
{
	trans1.set_rot((trans1.rot() + trans2.rot()) % 4);
	if (trans2.flip())
		trans1.set_flip(!trans1.flip());
}


// calculate trans1 -= trans2
inline void operator-=(Trans_IO &trans1, Trans_I trans2)
{
	trans1.set_rot((trans1.rot() - trans2.rot())%4);
	if (trans2.flip())
		trans1.set_flip(!trans1.flip());
}