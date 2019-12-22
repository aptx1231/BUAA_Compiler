#include <iostream>
#include <fstream>
#include "lexical.h"
#include "grammer.h"
#include "midCode.h"
#include "mipsCode.h"
#include "function.h"
#include "optimize.h"
using namespace std;

extern string filecontent;  //文件的内容
ifstream inputfile;
ofstream outputfile;
ofstream errorfile;
ofstream midCodefile;
ofstream mipsCodefile;
int debug = 1;
bool error = false;

int main() {
	inputfile.open("testfile.txt", ios::in);
	outputfile.open("output.txt", ios::out);
	errorfile.open("error.txt", ios::out);
	midCodefile.open("midCode.txt", ios::out);
	mipsCodefile.open("mips.txt", ios::out);
	string tmpIn;
	while (getline(inputfile, tmpIn)) {  //读取文件内容
		filecontent.append(tmpIn);
		filecontent.append("\n");
	}
	int re = getsym();
	if (re < 0) {
		//error()
	}
	else {
		if (procedure()) {
			//success
			//cout << "True!" << endl;
		}
		else {
			//error()
		}
	}
	if (!error) {
		//midCodefile.open("midCode.txt", ios::out);
		//mipsCodefile.open("mips.txt", ios::out);
		if (debug) {
			showFuncMidCode();
		}
		splitBlock();
		if (debug) {
			showFuncBlock();
		}
		outputMidCode();
		genMips();
		outputMipsCode();
		//midCodefile.close();
		//mipsCodefile.close();
		if (debug) {
			showGlobal();
			showAll();
			showString();
		}
	}
	inputfile.close();
	outputfile.close();
	errorfile.close();
	midCodefile.close();
	mipsCodefile.close();
	return 0;
}