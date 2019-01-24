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

class Trans;
typedef const Trans &Trans_I;
typedef Trans &Trans_O, &Trans_IO;

enum class Who : Char { NONE, WHITE, BLACK, DEFAULT, DRAW };
typedef const Who &Who_I;
typedef Who &Who_IO, &Who_O;

enum class Act : Char { PLACE, PASS, EDIT, INIT };
typedef const Act &Act_I;
typedef Act &Act_O, &Act_IO;

enum class Sol : Char { BAD, FAIR, GOOD, UNKNOWN, KO_GOOD, KO_BAD, KO_FAIR, FORBIDDEN};
typedef const Sol &Sol_I;
typedef Sol &Sol_O, &Sol_IO;

class Move;
typedef const Move &Move_I;
typedef Move &Move_O, &Move_IO;

class Node;
typedef const Node &Node_I;
typedef Node &Node_O, &Node_IO;

class Group;
typedef const Group &Group_I;
typedef Group &Group_O, &Group_IO;

class Config;
typedef const Config &Config_I;
typedef Config &Config_O, &Config_IO;

class Board;
typedef const Board & Board_I;
typedef Board &Board_O, &Board_IO;

class BoardRef;
typedef const BoardRef &BoardRef_I;
typedef BoardRef &BoardRef_O, &BoardRef_IO;

inline Bool is_ko_sol(Sol_I sol)
{
	if (sol == Sol::KO_GOOD || sol == Sol::KO_FAIR || sol == Sol::KO_BAD) {
		return true;
	}
	return false;
}

inline Sol sol2ko_sol(Sol_I sol)
{
	if (sol == Sol::GOOD)
		return Sol::KO_GOOD;
	else if (sol == Sol::FAIR)
		return Sol::KO_FAIR;
	else if (sol == Sol::BAD)
		return Sol::KO_BAD;
	else
		error("illegal sol!");
	return Sol::UNKNOWN;
}

// convert sol to integer
inline Int sol2int(Sol_I sol)
{
	if (sol == Sol::BAD)
		return 0;
	if (sol == Sol::FAIR)
		return 1;
	if (sol == Sol::GOOD)
		return 2;
	if (sol == Sol::UNKNOWN) {
		return -1; error("is this even useful?");
	}
	if (is_ko_sol(sol)) {
		return -2; error("is this even useful?");
	}
	error("illegal sol!");
}

inline std::ostream & operator<<(std::ostream &ostream, Sol sol)
{
	if (sol == Sol::GOOD) {
		return ostream << "good";
	}
	else if (sol == Sol::BAD) {
		return ostream << "bad";
	}
	else if (sol == Sol::FAIR) {
		return ostream << "fiar";
	}
	else
		error("unhandled case!");
}

inline Int operator-(Sol_I sol1, Sol_I sol2)
{
	return sol2int(sol1) - sol2int(sol2);
}

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
inline Who next(Who_I who)
{
	if (who == Who::BLACK) return Who::WHITE;
	if (who == Who::WHITE) return Who::BLACK;
	if (who == Who::NONE) return Who::BLACK; // empty board
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
#ifdef GOS_CHECK_BOUND
	if (territory2 > board_Nx() * board_Ny() * 2 || territory2 < 0)
		error("illegal territory!");
#endif
	return 2 * board_Nx()*board_Ny() - territory2;
}

// complementary score (opponent's territory)
inline Int inv_score2(Int_I score2)
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

// get inverse transform
inline Trans inv(Trans_I trans)
{
	Trans out = trans;
	if (out.rot() != 0)
		out.set_rot(4 - trans.rot());
	return out;
}

// transform coordinates
inline void transf(Char_IO x, Char_IO y, Int_I rot)
{
	Char temp, xmax = board_Nx() - 1, ymax = board_Ny() - 1;
	if (rot == 1) {
		temp = x; x = y; y = xmax - temp;
	}
	else if (rot == 2) {
		x = xmax - x; y = ymax-y;
	}
	else if (rot == 3) {
		temp = x; x = ymax - y; y = temp;
	}
}

// transform coordinates and who
inline void transf(Char_IO x, Char_IO y, Who_O who, Trans trans)
{
	Char temp, xmax = board_Nx() - 1, ymax = board_Ny() - 1;
	if (trans.rot() == 1) {
		temp = x; x = y; y = xmax-temp;
	}
	else if (trans.rot() == 2) {
		x = xmax-x; y = ymax-y;
	}
	else if (trans.rot() == 3) {
		temp = x; x = ymax-y; y = temp;
	}
	if (trans.flip())
		who = next(who);
}

// inverse transform coordinates
inline void inv_transf(Char_IO x, Char_IO y, Int_I rot)
{
	Char temp, xmax = board_Nx() - 1, ymax = board_Ny() - 1;
	if (rot == 1) {
		temp = x; x = ymax-y; y = temp;
	}
	else if (rot == 2) {
		x = xmax-x; y = ymax-y;
	}
	else if (rot == 3) {
		temp = x; x = y; y = xmax-temp;
	}
}

// inverse transform coordinates and who
inline void inv_transf(Char_IO x, Char_IO y, Who_O who, Trans trans)
{
	Char temp, xmax = board_Nx() - 1, ymax = board_Ny() - 1;
	if (trans.rot() == 1) {
		temp = x; x = ymax-y; y = temp;
	}
	else if (trans.rot() == 2) {
		x = xmax-x; y = ymax-y;
	}
	else if (trans.rot() == 3) {
		temp = x; x = y; y = xmax-temp;
	}
	if (trans.flip())
		who = next(who);
}

// calculate trans1 += trans2
inline void operator+=(Trans_IO trans1, Trans_I trans2)
{
	trans1.set_rot((trans1.rot() + trans2.rot()) % 4);
	if (trans2.flip())
		trans1.set_flip(!trans1.flip());
}

// calculate trans1 -= trans2
inline void operator-=(Trans_IO trans1, Trans_I trans2)
{
	trans1.set_rot(mod(trans1.rot() - trans2.rot(), 4));
	if (trans2.flip())
		trans1.set_flip(!trans1.flip());
}

inline Sol sco22sol(Int_I score2, Who_I who)
{
	Int score4_draw = board_Nx()*board_Ny() * 2;
	if (who == Who::BLACK)
		score4_draw += komi2();
	else if (who == Who::WHITE)
		score4_draw -= komi2();
	else
		error("illegal player!");
	Int score4 = 2 * score2;
	if (score4 > score4_draw)
		return Sol::GOOD;
	else if (score4 < score4_draw)
		return Sol::BAD;
	else
		return Sol::FAIR;
}

Who sol2winner(Who player, Sol sol)
{
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