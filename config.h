#pragma once
#include "group.h"
#include "SLISC/sort.h"

// board configuration, not situation
// for raw board, class RawBoard must be used
// origin at upper left corner, x axis points right, y axis points down
// TODO: implement operator << for move
// TODO: use a better comparation rule so that rotations of a board are considered equal, otherwise unequal
class Config
{
private:
	// === data member ===
	Matrix<Who> m_data;

public:
	// === constructors ===
	Config() {} // no data

	// === const functions ===

	// get a stone by coordinate
	Who operator()(Char_I x, Char_I y) const;

	Who & operator()(Char_I x, Char_I y);

	// display config on screen
	void disp() const;

	// check if a placing is legal, or how many stones will be dead
	// same check already exists for place()
	// Ko is not considered!
	// if legal, return the number of stones that can be removed ( >= 0)
	// return -1 if occupied, do nothing
	// return -2 if no qi, do nothing
	Int check(Char_I x, Char_I y, Who_I s) const;

	// get stones connected to a given stone (x,y) and calculate qi
	// if (x,y) does not have a stone yet, specifying "who" can assume a stone is placed
	// if (x,y) already have a stone, "who" argument is not used
	// "mark" will mark connected stones and their qi.
	// "group" has the coordinates of the connected stones
	void connect(MatChar_O mark, vector<Move> &qi, vector<Move> /*_O*/ &group,
		Char_I x, Char_I y, Who_I who = Who::DEFAULT) const;

	// internal recursive function called by connect()
	void connect0(MatChar_IO mark, vector<Move> &qi, vector<Move> /*_IO*/ &group,
		Char_I x, Char_I y, Who_I who = Who::DEFAULT) const;

	// get all qi's connected to one qi
	void connect_qi(MatChar_O mark, vector<Move> /*_O*/ &group, Char_I x, Char_I y) const;

	// internal recursive function called by connect_qi()
	void connect_qi0(MatChar_IO mark, vector<Move> /*_IO*/ &group, Char_I x, Char_I y) const;

	// remove a group of stone from the board
	void remove_group(const vector<Move> &group);

	// if the game has ended
	// game only ends when only eyes and mutual life (two qi's) are left
	Bool is_game_end() const;

	// calculate territory
	// this is only accurate when only dumb eye filling exists
	// X's territory = # X's stone on board + # single qi's surrounded by X + (other qi's not surrounded by Y)/2
	Int calc_territory2(Who_I who) const;

	// is (x,y) an eye surrounded by who
	Bool is_eye(Char_I x, Char_I y, Who_I who) const;

	// if filling of an eye that's not in danger
	Bool is_dumb_eye_filling(Char_I x, Char_I y, Who_I who) const;

	// if filling a big eye surrounded by connected stones
	Bool is_dumb_2eye_filling(Char_I x, Char_I y, Who_I who) const;

	// list all legal placing in random order
	void rand_legal_placing(vector<Move> &pos, Who_I who)
	{
		// TODO
	}

	// check atari for opponent group, generate all available moves and try to put the better moves first
	// for now, order the moves according to # of stones that can be removed
	// the moves only includes placing, not passing
	void eat_list(vector<Move> &pos, Who_I who) const
	{
		using slisc::sort2;
		Int i, Nqi, Ndead, Ngroup;
		vector<Group> groups;
		vector<Int> dead_group_inds;
		vector<Int> dead_group_size;

		pos.resize(0);
		all_groups(groups, ::next(who));
		Ngroup = groups.size();

		for (i = 0; i < groups.size(); ++i) {
			Nqi = groups[i].qi().size();
			if (Nqi == 1) {
				dead_group_inds.push_back(i);
				dead_group_size.push_back(Nqi);
			}
		}

		Ndead = dead_group_inds.size();
		
		if (Ndead == 0)
			return;
		else if (Ndead == 1) {
			pos.push_back(groups[dead_group_inds[0]].qi()[0]);
			return;
		}
		else {
			// more than one dead group
			sort2(dead_group_size, dead_group_inds); // ascending order
			pos.resize(Ndead);
			for (i = Ndead - 1; i >= 0; --i) {
				pos[i] = groups[dead_group_inds[i]].qi()[0];
			}
		}
	}

	// find all groups for a player
	void all_groups(vector<Group> &groups, Who_I who) const
	{
		Char x, y, Nx = board_Nx(), Ny = board_Ny();
		Int i;
		MatChar tot_mark(Nx, Ny, Char(0)), mark;
		Group group;
		for (y = 0; y < Ny; ++y) {
			for (x = 0; x < Nx; ++x) {
				if (m_data(x, y) == who && !tot_mark(x, y)) {
					groups.push_back(Group());
					vector<Move> &qi = groups.back().qi();
					vector<Move> &group = groups.back().pos();
					connect(mark, qi, group, x, y, who);
					for (i = 0; i < group.size(); ++i) {
						tot_mark(group[i].x(), group[i].y()) = 1;
					}
				}
			}
		}
	}

	// decide the best transformations (rotation and stone color flipping) to make a configuration "largest"
	// NONE:0 < WHITE:1 < BLACK:2
	// return true: if needs to flip
	// return false: if no need to flip
	// output rot: times the board needs to rotate counter-clockwise
	Trans calc_trans() const;

	// get a stone after the board has been rotated and fliped
	Who transform1(Char_I x, Char_I y, Trans_I trans) const;

	// transform the board to a new board
	void transform(Config_O board, Trans_I trans) const;

	// === none-const functions ===

	void init(); // init to an empty board

	// move internal data from one config to another without copying
	void operator<<(Config_IO rhs);

	// transform a board itself
	void transform(Trans_I trans);

	// return 0 if legal, and update board
	// Ko is not considered! 
	// return -1 if occupied, do nothing
	// return -2 if no qi, do nothing
	Int place(Char_I x, Char_I y, Who_I s);
};

// === interface functions ===

// compare two boards
// return 1: board > board_raw
// return 0: board == board_raw
// return -1: board < board_raw
// transformation will be performed on "board_raw", but not "board"
inline Int operator-(Config_I config1, Config_I config2);

// === implementation ===

inline Who Config::operator()(Char_I x, Char_I y) const
{
	return m_data(x, y);
}

inline Who & Config::operator()(Char_I x, Char_I y)
{
	return m_data(x, y);
}

inline void Config::disp() const
{
	Char i, x, y, Nx = board_Nx(), Ny = board_Ny();
	cout << "     ";
	for (i = 0; i < board_Nx(); ++i) cout << Int(i) << "   "; cout << "\n";
	cout << "    ";
	for (x = 0; x < Nx; ++x) cout << "----"; cout << "\n";
	for (y = 0; y < Ny; ++y) {
		cout << " " << Int(y) << " |";
		for (x = 0; x < Nx; ++x) {
			if (m_data(x, y) == Who::NONE)
				cout << "   |";
			else if (m_data(x, y) == Who::BLACK)
				cout << " @ |";
			else if (m_data(x, y) == Who::WHITE)
				cout << " O |";
			else
				error("Config::disp(): illegal stone code!");
		}
		cout << "\n";
		cout << "    ";
		for (i = 1; i <= Nx; ++i) cout << "----"; cout << "\n";
	}
}

inline Trans Config::calc_trans() const
{
	Char x, y;
	Int i, Nx = board_Nx(), Ny = board_Ny(), N = Nx * Ny, best;
	vector<Int> val;
	vector<Bool>;
	vector<Trans> trans;

	// square board (4 rotations)
	if (Nx == Ny) {
		trans.resize(8); val.resize(8);
		trans[0].set_flip(false); trans[0].set_rot(0);
		trans[1].set_flip(false); trans[1].set_rot(1);
		trans[2].set_flip(false); trans[2].set_rot(2);
		trans[3].set_flip(false); trans[3].set_rot(3);
		trans[4].set_flip(true);  trans[4].set_rot(0);
		trans[5].set_flip(true);  trans[5].set_rot(1);
		trans[6].set_flip(true);  trans[6].set_rot(2);
		trans[7].set_flip(true);  trans[7].set_rot(3);
	}
	// rectangle board (2 rotations)
	else {
		trans.resize(4); val.resize(4);
		trans[0].set_flip(false); trans[0].set_rot(0);
		trans[1].set_flip(false); trans[1].set_rot(2);
		trans[2].set_flip(true);  trans[2].set_rot(0);
		trans[3].set_flip(true);  trans[3].set_rot(2);
	}

	// check board coordinates in row-major order
	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			// evaluate (x, y) position for all transformations left
			best = 0;
			for (i = 0; i < val.size(); ++i) {
				val[i] = who2int(transform1(x, y, trans[i]));
				if (best < val[i])
					best = val[i];
			}

			// only keep the best transformations
			for (i = 0; i < val.size(); ++i) {
				if (val[i] < best) {
					trans.erase(trans.begin() + i);
					val.erase(val.begin() + i);
					--i;
				}
			}

			// only one transformations left
			if (val.size() == 1) {
				return trans[0];
			}
		}
	}

	// multiple rot/flip combinations, return the first
	return trans[0];
}

inline Who Config::transform1(Char_I x, Char_I y, Trans_I trans) const
{
	Char x1, y1, xmax = board_Nx() - 1, ymax = board_Ny() - 1;
	Who stone;
	if (trans.rot() == 0) {
		x1 = x; y1 = y;
	}
	else if (trans.rot() == 1) {
		x1 = ymax - y; y1 = x;
	}
	else if (trans.rot() == 2) {
		x1 = xmax - x; y1 = ymax - y;
	}
	else if (trans.rot() == 3) {
		x1 = y; y1 = xmax - x;
	}
	else
		error("illegal rotation!");

	stone = m_data(x1, y1);
	if (trans.flip()) {
		if (stone == Who::WHITE)
			return Who::BLACK;
		if (stone == Who::BLACK)
			return Who::WHITE;
	}

	return stone;
}

void Config::transform(Config_O config, Trans_I trans) const
{
	Char Nx = board_Nx(), Ny = board_Ny(), x, y;
	config.init();
	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			config(x, y) = transform1(x, y, trans);
		}
	}
}

// TODO this might be a slow implementation
void Config::transform(Trans_I trans)
{
	Char x1, y1, x, y, Nx = board_Nx(), Ny = board_Ny();
	Who who1;
	Config config;
	transform(config, trans);
	*this << config;
}

inline Int Config::check(Char_I x, Char_I y, Who_I who) const
{
#ifdef GOS_CHECK_BOUND
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Tree::place(x,y): out of bound!");
#endif
	Int Nx = board_Nx(), Ny = board_Ny(), Nremove = 0;

	// output of connect()
	MatChar mark(Nx, Ny, Char(0)); vector<Move> group; vector<Move> qi;

	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return -1;

	// search opponent's dead stones
	// only necessary if placed next to opposite stone
	if (x > 0 && m_data(x - 1, y) == next(who)) {
		connect(mark, qi, group, x - 1, y);
		if (qi.size() == 1)
			Nremove += group.size();
	}

	if (y > 0 && m_data(x, y - 1) == next(who) && !mark(x, y - 1)) {
		connect(mark, qi, group, x, y - 1);
		if (qi.size() == 1)
			Nremove += group.size();
	}

	if (x < Nx - 1 && m_data(x + 1, y) == next(who) && !mark(x + 1, y)) {
		connect(mark, qi, group, x + 1, y);
		if (qi.size() == 1)
			Nremove += group.size();
	}

	if (y < Ny - 1 && m_data(x, y + 1) == next(who) && !mark(x, y + 1)) {
		connect(mark, qi, group, x, y + 1);
		if (qi.size() == 1)
			Nremove += group.size();
	}

	if (Nremove > 0)
		return Nremove;

	// check qi assuming stone is placed
	connect(mark, qi, group, x, y, who);
	if (qi.size() == 0) return -2;

	return 0;
}

inline void Config::init()
{
	m_data.resize(board_Nx(), board_Ny());
	m_data = Who::NONE;
}

inline Int Config::place(Char_I x, Char_I y, Who_I who)
{
#ifdef GOS_CHECK_BOUND
	if (x < 0 || y < 0 || x >= board_Nx() || y >= board_Ny())
		error("Tree::place(x,y): out of bound!");
#endif
	Int Nx = board_Nx(), Ny = board_Ny();
	Bool removed = false;

	// output of connect()
	MatChar mark(board_Nx(), board_Ny(), Char(0)); vector<Move> group; vector<Move> qi;

	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return -1;

	// place stone
	m_data(x, y) = who;

	// remove opponent's dead stones
	// only necessary if placed next to opposite stone
	if (x > 0 && m_data(x - 1, y) == next(who)) {
		connect(mark, qi, group, x - 1, y);
		if (qi.size() == 0) {
			remove_group(group); removed = true;
		}
	}

	if (y > 0 && m_data(x, y - 1) == next(who) && !mark(x, y - 1)) {
		connect(mark, qi, group, x, y - 1);
		if (qi.size() == 0) {
			remove_group(group); removed = true;
		}
	}

	if (x < Nx - 1 && m_data(x + 1, y) == next(who) && !mark(x + 1, y)) {
		connect(mark, qi, group, x + 1, y);
		if (qi.size() == 0) {
			remove_group(group); removed = true;
		}
	}

	if (y < Ny - 1 && m_data(x, y + 1) == next(who) && !mark(x, y + 1)) {
		connect(mark, qi, group, x, y + 1);
		if (qi.size() == 0) {
			remove_group(group); removed = true;
		}
	}

	if (removed) return 0;

	// check qi of placed stone
	connect(mark, qi, group, x, y);
	if (qi.size() == 0) return -2;

	return 0;
}

void Config::operator<<(Config_IO rhs)
{
	m_data << rhs.m_data;
}

inline void Config::connect(MatChar_O mark, vector<Move> &qi, vector<Move> /*_O*/ &group,
	Char_I x, Char_I y, Who_I who_assume /*optional*/) const
{
	// init output
	mark.resize(board_Nx(), board_Ny()); mark = 0; group.resize(0); qi.resize(0);
	// connect (x,y) recursively
	connect0(mark, qi, group, x, y, who_assume);
}

inline void Config::connect0(MatChar_IO mark, vector<Move> &qi, vector<Move> /*_IO*/ &group,
	Char_I x, Char_I y, Who_I who_assume) const
{
	Char Nx = board_Nx(), Ny = board_Ny();
	Who who, who0;
	if (m_data(x, y) == Who::NONE) {
		if (who_assume == Who::DEFAULT)
			error("must specify who_assume argument!");
		else
			who0 = who_assume;
	}
	else
		who0 = m_data(x, y);

	group.push_back(Move(x, y));
	mark(x, y) = 1;

	if (x > 0) {
		who = m_data(x - 1, y);
		Char & rmark = mark(x - 1, y);
		if (who == who0 && !rmark)
			connect0(mark, qi, group, x - 1, y);
		else if (who == Who::NONE && !rmark) {
			qi.push_back(Move(x - 1, y)); rmark = 1;
		}
	}

	if (x < Nx - 1) {
		who = m_data(x + 1, y);
		Char & rmark = mark(x + 1, y);
		if (who == who0 && !rmark)
			connect0(mark, qi, group, x + 1, y);
		else if (who == Who::NONE && !rmark) {
			qi.push_back(Move(x + 1, y)); rmark = 1;
		}
	}

	if (y > 0) {
		who = m_data(x, y - 1);
		Char & rmark = mark(x, y - 1);
		if (who == who0 && !rmark)
			connect0(mark, qi, group, x, y - 1);
		else if (who == Who::NONE && !rmark) {
			qi.push_back(Move(x, y - 1)); rmark = 1;
		}
	}

	if (y < Ny - 1) {
		who = m_data(x, y + 1);
		Char & rmark = mark(x, y + 1);
		if (who == who0 && !rmark)
			connect0(mark, qi, group, x, y + 1);
		else if (who == Who::NONE && !rmark) {
			qi.push_back(Move(x, y + 1)); rmark = 1;
		}
	}
}

void Config::connect_qi(MatChar_O mark, vector<Move> /*_O*/ &group, Char_I x, Char_I y) const
{
	// init output
	mark.resize(board_Nx(), board_Ny()); mark = Char(0); group.resize(0);
	// connect (x,y) recursively
	connect_qi0(mark, group, x, y);
}

void Config::connect_qi0(MatChar_IO mark, vector<Move> /*_IO*/ &group, Char_I x, Char_I y) const
{
	Char Nx = board_Nx(), Ny = board_Ny(), x1, y1;

	if (m_data(x, y) != Who::NONE)
		error("(x,y) has a stone!");

	group.push_back(Move(x, y));
	mark(x, y) = 1;

	if (x > 0) {
		x1 = x - 1; y1 = y;
		if (m_data(x1, y1) == Who::NONE && !mark(x1, y1)) {
			connect_qi0(mark, group, x1, y1);
		}
	}

	if (x < Nx - 1) {
		x1 = x + 1; y1 = y;
		if (m_data(x1, y1) == Who::NONE && !mark(x1, y1)) {
			connect_qi0(mark, group, x1, y1);
		}
	}

	if (y > 0) {
		x1 = x; y1 = y - 1;
		if (m_data(x1, y1) == Who::NONE && !mark(x1, y1)) {
			connect_qi0(mark, group, x1, y1);
		}
	}

	if (y < Ny - 1) {
		x1 = x; y1 = y + 1;
		if (m_data(x1, y1) == Who::NONE && !mark(x1, y1)) {
			connect_qi0(mark, group, x1, y1);
		}
	}
}

inline void Config::remove_group(const vector<Move> &group)
{
	Int i;
	for (i = 0; i < group.size(); ++i)
		m_data(group[i].x(), group[i].y()) = Who::NONE;
}

Bool Config::is_game_end() const
{
	Char Nx = board_Nx(), Ny = board_Ny(), x, y;
	Int qi, i;
	MatChar tot_mark(Nx, Ny, Char(0)), mark;
	vector<Move> group;

	// scan all qi's on the board
	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			if (m_data(x, y) != Who::NONE && !tot_mark(x, y))
				continue;
			connect_qi(mark, group, x, y);
			qi = group.size();
			for (i = 0; i < qi; ++i) {
				tot_mark(group[i].x(), group[i].y()) = 1;
			}
			if (qi > 2)
				return false;
			else if (qi == 1) {
				if (is_eye(x, y, Who::BLACK) || is_eye(x, y, Who::WHITE))
					continue;
				else
					return false;
			}
			else if (qi == 2) {
				if (is_dumb_2eye_filling(x, y, Who::BLACK) || is_dumb_2eye_filling(x, y, Who::WHITE))
					return false;
				cout << "warning: check mutual life for large board, ignore for now" << endl;
				cout << "warning: check mutual life for large board, ignore for now" << endl;;
				cout << "warning: check mutual life for large board, ignore for now" << endl;;
				cout << "warning: check mutual life for large board, ignore for now" << endl;;
				return false;
			}
		}
	}
	return true;
}

inline Int Config::calc_territory2(Who_I who) const
{
	Char x, y, Nx = board_Nx(), Ny = board_Ny();
	Int black = 0, qi = 0, common_qi = 0;
	Who s;
	for (x = 0; x < Nx; ++x)
		for (y = 0; y < Ny; ++y) {
			s = m_data(x, y);
			if (s == Who::NONE) {
				// qi not surrounded by black
				if ((x > 0 && m_data(x - 1, y) != who)
					|| (x < Nx - 1 && m_data(x + 1, y) != who)
					|| (y > 0 && m_data(x, y - 1) != who)
					|| (y < Ny - 1 && m_data(x, y + 1) != who)) {
					// qi not surrounded by white
					if ((x > 0 && m_data(x - 1, y) != next(who))
						|| (x < Nx - 1 && m_data(x + 1, y) != next(who))
						|| (y > 0 && m_data(x, y - 1) != next(who))
						|| (y < Ny - 1 && m_data(x, y + 1) != next(who)))
						++common_qi;
				}
				else // qi surrounded by black
					++qi;
			}
			else if (s == who)
				++black;
			else if (s != next(who))
				error("Config::result(): illegal stone!");
		}
	return 2 * (black + qi) + common_qi;
}

inline Bool Config::is_eye(Char_I x, Char_I y, Who_I who) const
{
	// check if already occupied
	if (m_data(x, y) != Who::NONE)
		return false;

	// check if not surrounded by the same color or board
	if (x > 0 && m_data(x - 1, y) != who ||
		x < board_Nx() - 1 && m_data(x + 1, y) != who ||
		y > 0 && m_data(x, y - 1) != who ||
		y < board_Ny() - 1 && m_data(x, y + 1) != who)
		return false;

	return true;
}

inline Bool Config::is_dumb_eye_filling(Char_I x, Char_I y, Who_I who) const
{
	// not my eye 
	if (!is_eye(x, y, who)) return false;

	Int Nx = board_Nx(), Ny = board_Ny();

	// output of connect()
	MatChar mark(Nx, Ny); vector<Move> group; vector<Move> qi;

	// check qi of sourrounding 4 stones
	// if only 1 qi, it's not dumb

	if (x > 0) {
		connect(mark, qi, group, x - 1, y);
		if (qi.size() == 1) return false;
	}

	if (y > 0 && !mark(x, y - 1)) {
		connect(mark, qi, group, x, y - 1);
		if (qi.size() == 1) return false;
	}

	if (x < Nx - 1 && !mark(x + 1, y)) {
		connect(mark, qi, group, x + 1, y);
		if (qi.size() == 1) return false;
	}

	if (y < Ny - 1 && !mark(x, y + 1)) {
		connect(mark, qi, group, x, y + 1);
		if (qi.size() == 1) return false;
	}

	// it is dumb...
	return true;
}

inline Bool Config::is_dumb_2eye_filling(Char_I x, Char_I y, Who_I who) const
{
	Int Nx = board_Nx(), Ny = board_Ny();
	Int black;
	Char x1, y1, x_qi, y_qi;
	Bool found_qi = false, connected = false;

	// output of connect()
	MatChar mark(Nx, Ny); vector<Move> group; vector<Move> qi;

	// check qi of sourrounding 3 stones and one qi

	// check left
	x1 = x - 1; y1 = y;
	if (x > 0) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!mark(x1, y1)) return false; // black not connected
			}
			else {
				connect(mark, qi, group, x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (found_qi) return false; // more than one qi
			found_qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check right
	x1 = x + 1; y1 = y;
	if (x < Nx - 1) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!mark(x1, y1)) return false; // black not connected
			}
			else {
				connect(mark, qi, group, x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (found_qi) return false; // more than one qi
			found_qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check up
	x1 = x; y1 = y - 1;
	if (y > 0) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!mark(x1, y1)) return false; // black not connected
			}
			else {
				connect(mark, qi, group, x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (found_qi) return false; // more than one qi
			found_qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check down
	x1 = x; y1 = y + 1;
	if (y < Ny - 1) {
		if (m_data(x1, y1) == who) {
			if (connected) {
				if (!mark(x1, y1)) return false; // black not connected
			}
			else {
				connect(mark, qi, group, x1, y1);
				connected = true;
			}
		}
		else if (m_data(x1, y1) == Who::NONE) {
			if (found_qi) return false; // more than one qi
			found_qi = true; x_qi = x1; y_qi = y1;
		}
		else
			return false; // found white stone
	}

	// check the other qi

	// left
	x1 = x_qi - 1; y1 = y_qi;
	if (x_qi > 0 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && mark(x1, y1)))
			return false;
	}

	// right
	x1 = x_qi + 1; y1 = y_qi;
	if (x_qi < Nx - 1 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && mark(x1, y1)))
			return false;
	}

	// up
	x1 = x_qi; y1 = y_qi - 1;
	if (y_qi > 0 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && mark(x1, y1)))
			return false;
	}

	// down
	x1 = x_qi; y1 = y_qi + 1;
	if (y_qi < Ny - 1 && !(x1 == x && y1 == y)) {
		if (!(m_data(x1, y1) == who && mark(x1, y1)))
			return false;
	}

	// ok, it is dumb...
	return true;
}

inline Int operator-(Config_I config1, Config_I config2)
{
	// rotation and inversion should make the board compare as big as possible
	Char x, y;
	Int Nx = board_Nx(), Ny = board_Ny(), N = Nx * Ny;
	Int val1, val2;

	for (y = 0; y < Ny; ++y) {
		for (x = 0; x < Nx; ++x) {
			val1 = who2int(config1(x, y));
			val2 = who2int(config2(x, y));
			if (val1 == val2) continue;
			if (val1 > val2) return 1;
			return -1;
		}
	}
	return false;
}
