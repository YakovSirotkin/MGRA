/*
 * nucl.cpp
 *
 *  Created on: 01.03.2011
 *      Author: vyahhi
 */

#include "nucl.hpp"

char complement(char c) {
	return c ^ 3;
}

char nucl(char c) {
	switch(c) {
		case 0: return 'A';
		case 1: return 'C';
		case 2: return 'G';
		case 3: return 'T';
		default: return 'N';
	}
}

char unnucl(char c) {
	switch(c) {
		case 'C': case '1': case 1: return 1;
		case 'G': case '2': case 2: return 2;
		case 'T': case '3': case 3: return 3;
		default: return 0;
	}
}