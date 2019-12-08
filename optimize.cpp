#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "midCode.h"
#include "optimize.h"
using namespace std;

extern vector<midCode> midCodeTable;  //所有的中间代码
extern map<string, vector<midCode> > funcMidCodeTable;  //每个函数单独的中间代码
map<string, vector<Block> > funcBlockTable;   //每个函数的基本块列表

void splitBlock() {
	for (map<string, vector<midCode> >::iterator it = funcMidCodeTable.begin(); it != funcMidCodeTable.end(); it++) {
		string funcName = (*it).first;
		vector<midCode> mcVe = (*it).second;
		set<int> split;
		vector<int> splitVe;
		map<string, int> label2poi;
		for (int i = 0; i < mcVe.size(); i++) {
			midCode mc = mcVe[i];
			if (mc.op == FUNC) {
				split.insert(i);
			}
			else if (mc.op == LABEL) {
				split.insert(i);
				label2poi[mc.z] = i;
			}
			else if (mc.op == BZ || mc.op == BNZ || mc.op == GOTO || mc.op == RET || mc.op == EXIT) {
				split.insert(i + 1);  //i+1<ve.size()?
			}
		}
		split.insert(mcVe.size()); //!!!!要不然最后一个label不成块
		for (set<int>::iterator it = split.begin(); it != split.end(); it++) {
			cout << (*it) << " ";
		}
		cout << "\n";
		splitVe.assign(split.begin(), split.end());
		vector<Block> blockVe;
		for (int i = 0; i + 1 < splitVe.size(); i++) {
			Block bl = Block(splitVe[i], splitVe[i + 1] - 1, -1, -1);
			if (mcVe[splitVe[i + 1] - 1].op != RET && mcVe[splitVe[i + 1] - 1].op != EXIT) { //ret没有后继1,2
				bl.setnextBlock1(splitVe[i + 1]);
			}
			if (mcVe[splitVe[i + 1] - 1].op == BZ || mcVe[splitVe[i + 1] - 1].op == BNZ || mcVe[splitVe[i + 1] - 1].op == GOTO) {
				bl.setnextBlock2(label2poi[mcVe[splitVe[i + 1] - 1].z]);
			}
			for (int j = bl.start; j <= bl.end; j++) {
				bl.insert(mcVe[j]);
			}
			blockVe.push_back(bl);
		}
		funcBlockTable.insert(make_pair(funcName, blockVe));
	}
}

void showFuncBlock() {
	for (map<string, vector<Block> >::iterator it = funcBlockTable.begin(); it != funcBlockTable.end(); it++) {
		string funcName = (*it).first;
		vector<Block> blockVe = (*it).second;
		cout << funcName << ":\n";
		for (int i = 0; i < blockVe.size(); i++) {
			blockVe[i].output();
		}
	}
}