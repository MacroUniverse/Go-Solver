#pragma once
#include "slisc.h"
#include "time.h"

namespace slisc {
	// input a bool
	// " (y/n) " will be appended to prompt
	Bool inpBool(const std::string &prompt) {
		Char y_n;
		for (Int j = 0; j < 10000; ++j) {
			cout << prompt << " (y/n) ";
			cin >> y_n;
			if (y_n == 'y') {
				return true;
			}
			else if (y_n == 'n') {
				return false;
			}
			else {
				warning("illegal input, try again!");
				pause(1);
			}
		}
	}

	void inpUint2(Int_O &i1, Int_O &i2, const std::string &prompt)
	{
		for (Int j = 0; j < 10000; ++j) {
			cout << prompt << " ";
			cin >> i1 >> i2;
			if (i1 < 0 || i2 < 0) {
				warning("illegal input, try again!");
				pause(1);
			}
		}
	}
}
