#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "midCode.h"
#include "optimize.h"
#include "symbolItem.h"
using namespace std;

extern map<string, map<string, symbolItem>> allLocalSymbolTable;
extern vector<midCode> midCodeTable;  //所有的中间代码
extern map<string, vector<midCode> > funcMidCodeTable;  //每个函数单独的中间代码
map<string, vector<Block> > funcBlockTable;   //每个函数的基本块列表

void calUseDef(Block& bl, string funcName) {
	set<string> use, def;
	for (int i = 0; i < bl.midCodeVector.size(); i++) {
		midCode mc = bl.midCodeVector[i];
		switch (mc.op) {
		case PLUSOP:
		case MINUOP:
		case MULTOP:
		case DIVOP:
			if (allLocalSymbolTable[funcName].find(mc.x) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.x].kind == 1 && mc.x[0] != '#') {  //局部变量
				if (use.find(mc.x) == use.end() && def.find(mc.x) == def.end()) {
					use.insert(mc.x);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.y) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.y].kind == 1 && mc.y[0] != '#') {  //局部变量
				if (use.find(mc.y) == use.end() && def.find(mc.y) == def.end()) {
					use.insert(mc.y);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
				if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
					def.insert(mc.z);
				}
			}
			break;
		case LSSOP:
		case LEQOP:
		case GREOP:
		case GEQOP:
		case EQLOP:
		case NEQOP:
			if (allLocalSymbolTable[funcName].find(mc.x) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.x].kind == 1 && mc.x[0] != '#') {  //局部变量
				if (use.find(mc.x) == use.end() && def.find(mc.x) == def.end()) {
					use.insert(mc.x);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.y) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.y].kind == 1 && mc.y[0] != '#') {  //局部变量
				if (use.find(mc.y) == use.end() && def.find(mc.y) == def.end()) {
					use.insert(mc.y);
				}
			}
			break;
		case ASSIGNOP:
		case GETARRAY:
			if (allLocalSymbolTable[funcName].find(mc.x) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.x].kind == 1 && mc.x[0] != '#') {  //局部变量
				if (use.find(mc.x) == use.end() && def.find(mc.x) == def.end()) {
					use.insert(mc.x);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
				if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
					def.insert(mc.z);
				}
			}
			break;
		case PUSH:
		case RET:
		case INLINERET:
			if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
				if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
					use.insert(mc.z);
				}
			}
			break;
		case RETVALUE:
		case SCAN:
			if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
				if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
					def.insert(mc.z);
				}
			}
			break;
		case PRINT:
			if (mc.x == "1" || mc.x == "2") {
				if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
					&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
					if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
						use.insert(mc.z);
					}
				}
			}
			cout << "PRINT " << mc.z << " " << mc.x << "\n";
			break;
		case PUTARRAY:
			if (allLocalSymbolTable[funcName].find(mc.y) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.y].kind == 1 && mc.y[0] != '#') {  //局部变量
				if (use.find(mc.y) == use.end() && def.find(mc.y) == def.end()) {
					use.insert(mc.y);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.z) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.z].kind == 1 && mc.z[0] != '#') {  //局部变量
				if (use.find(mc.z) == use.end() && def.find(mc.z) == def.end()) {
					def.insert(mc.z);
				}
			}
			break;
		case CONST:
		case ARRAY:
		case VAR:
		case PARAM:
		case FUNC:
		case EXIT:
		case GOTO:
		case BZ:
		case BNZ:
		case CALL:
		case LABEL:
			break;
		default:
			break;
		}
	}
	for (set<string>::iterator it = use.begin(); it != use.end(); it++) {
		bl.useInsert((*it));
	}
	for (set<string>::iterator it = def.begin(); it != def.end(); it++) {
		bl.defInsert((*it));
	}
}

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
			calUseDef(bl, funcName);
			blockVe.push_back(bl);
		}
		//把nextBlock改成Block的id 而不是Block的start
		for (int i = 0; i < blockVe.size(); i++) {
			if (blockVe[i].nextBlock1 != -1) {
				if (blockVe[i].nextBlock1 == mcVe.size()) {
					blockVe[i].nextBlock1 = -1;
				}
				else {
					for (int j = 0; j < blockVe.size(); j++) {
						if (blockVe[i].nextBlock1 == blockVe[j].start) {
							blockVe[i].nextBlock1 = j;
							break;
						}
					}
				}
			}
			if (blockVe[i].nextBlock2 != -1) {
				if (blockVe[i].nextBlock2 == mcVe.size()) {
					blockVe[i].nextBlock2 = -1;
				}
				else {
					for (int j = 0; j < blockVe.size(); j++) {
						if (blockVe[i].nextBlock2 == blockVe[j].start) {
							blockVe[i].nextBlock2 = j;
							break;
						}
					}
				}
			}
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