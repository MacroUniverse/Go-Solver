#pragma once
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#ifndef NDEBUG
#define _RAND_SEED_ 0
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
