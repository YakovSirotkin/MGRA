/*
 * preproc.cpp
 *
 *  Created on: 11.05.2011
 *      Author: snikolenko
 */
 
#include<cmath>
#include<string>
#include<iostream>
#include<sstream>
#include<fstream>
#include<cstdlib>
#include<vector>
#include<map>
#include<list>
#include<queue>
#include<cstdarg>
#include<algorithm>
#include<cassert>
#include "defs.hpp"

using namespace std;
ostringstream oMsg;

int qvoffset;


double oct2phred(string qoct)  {

	float freq = 1;	
	for (size_t i = 0; i < qoct.length(); i++) {
		freq *= 1 - pow(10, -float(qoct[i] - qvoffset)/10.0);
	}

	return freq;
}               

void trimbs(string & seq, string & qv, int starttrim, int endtrim) {
	assert(seq.length() == qv.length());
	if (endtrim == -1) endtrim = qv.length();
	endtrim = min(endtrim, (int) qv.length());
	for (int i = starttrim; i < endtrim; i++) {
		if (qv[i] != 'B') break; else starttrim++; 
	}
	for (int i = endtrim - 1; i >= 0;  i--) {
		if (qv[i] != 'B') break; else endtrim--;
	}
	
	if (endtrim > starttrim) {
		seq = seq.substr(starttrim, endtrim - starttrim  );
		qv = qv.substr(starttrim, endtrim - starttrim  );
	} else {
		seq = ""; 
		qv  = "";
	}
	return;
}

int main(int argc, char * argv[]) {
	int starttrim = atoi(argv[1]); 
	int endtrim = atoi(argv[2]);  //number of characters to keep
	qvoffset = atoi(argv[3]);

	string tag;
	string qualoct;

	string header, seq, junk, qv;
	while (getline(cin, header)) {
		getline(cin, seq);
		getline(cin, junk);
		getline(cin, qv);
		trimbs(seq, qv, starttrim, endtrim);
		if (seq != "") cout << seq << "\t" << oct2phred(qv)<< endl;
	}
}

