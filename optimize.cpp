#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <queue>
#include "midCode.h"
#include "optimize.h"
#include "symbolItem.h"
using namespace std;

extern map<string, map<string, symbolItem>> allLocalSymbolTable;
extern vector<midCode> midCodeTable;  //所有的中间代码
extern map<string, vector<midCode> > funcMidCodeTable;  //每个函数单独的中间代码
map<string, vector<Block> > funcBlockTable;   //每个函数的基本块列表
extern int debug;

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
		case GETARRAY:  //mc.z << " = " << mc.x << "[" << mc.y << "]
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
			break;
		case PUTARRAY:  //mc.z << "[" << mc.x << "]" << " = " << mc.y
			if (allLocalSymbolTable[funcName].find(mc.y) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.y].kind == 1 && mc.y[0] != '#') {  //局部变量
				if (use.find(mc.y) == use.end() && def.find(mc.y) == def.end()) {
					use.insert(mc.y);
				}
			}
			if (allLocalSymbolTable[funcName].find(mc.x) != allLocalSymbolTable[funcName].end()
				&& allLocalSymbolTable[funcName][mc.x].kind == 1 && mc.x[0] != '#') {  //局部变量
				if (use.find(mc.x) == use.end() && def.find(mc.x) == def.end()) {
					use.insert(mc.x);
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
		//for (set<int>::iterator it = split.begin(); it != split.end(); it++) {
		//	cout << (*it) << " ";
		//}
		//cout << "\n";
		splitVe.assign(split.begin(), split.end());
		vector<Block> blockVe;
		for (int i = 0; i + 1 < splitVe.size(); i++) {
			Block bl = Block(splitVe[i], splitVe[i + 1] - 1, -1, -1);
			//RET,EXIT没有后继1,2  GOTO只有后继2 BZ,BNZ两个后继都有
			if (mcVe[splitVe[i + 1] - 1].op != RET && mcVe[splitVe[i + 1] - 1].op != EXIT
				&& mcVe[splitVe[i + 1] - 1].op != GOTO) {
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
	calInOut();
	for (map<string, vector<Block> >::iterator it = funcBlockTable.begin(); it != funcBlockTable.end(); it++) {
		string funcName = (*it).first;
		vector<Block> blockVe = (*it).second;
		if (debug) {
			cout << "-----------------" << funcName << "--------------------\n";
		}
		for (map<string, symbolItem>::iterator iter = allLocalSymbolTable[funcName].begin();
			iter != allLocalSymbolTable[funcName].end(); iter++) {  //看这个函数有哪些变量
			symbolItem sitem = (*iter).second;
			if (sitem.kind == 1 && sitem.name[0] != '#' && sitem.name[0] != '%') {  //var
				if (debug) {
					cout << "-----------" << (*iter).first << "-------------\n";
				}
				string name = (*iter).first;
				for (int i = 0; i < blockVe.size(); i++) {
					//name在块i的def集合中
					if (find(blockVe[i].def.begin(), blockVe[i].def.end(), name) != blockVe[i].def.end()) {
						int islive = false;
						int defPoi = -1;
						for (int l = blockVe[i].midCodeVector.size() - 1; l >= 0; l--) {
							midCode mc = blockVe[i].midCodeVector[l];
							switch (mc.op) {
							case PLUSOP:
							case MINUOP:
							case MULTOP:
							case DIVOP:
							case LSSOP:
							case LEQOP:
							case GREOP:
							case GEQOP:
							case EQLOP:
							case NEQOP:
							case PUTARRAY:  //mc.z << "[" << mc.x << "]" << " = " << mc.y
								if (mc.x == name || mc.y == name) {
									islive = true;
								}
								if (mc.z == name) {
									defPoi = l;
								}
								break;
							case ASSIGNOP:
								if (mc.x == name) {
									islive = true;
								}
								if (mc.z == name) {
									defPoi = l;
								}
								break;
							case GETARRAY:  //mc.z << " = " << mc.x << "[" << mc.y << "]
								if (mc.y == name) {
									islive = true;
								}
								if (mc.z == name) {
									defPoi = l;
								}
								break;
							case PUSH:
							case RET:
							case INLINERET:
								if (mc.z == name) {
									islive = true;
								}
								break;
							case RETVALUE:
							case SCAN:
								if (mc.z == name) {
									defPoi = l;
								}
								break;
							case PRINT:
								if (mc.x == "1" || mc.x == "2") {
									if (mc.z == name) {
										islive = true;
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
							default:
								break;
							}
							if (islive || defPoi != -1) {
								break;
							}
						}
						if (!islive) {
							for (int j = 0; j < blockVe.size(); j++) {
								if (find(blockVe[j].in.begin(), blockVe[j].in.end(), name) != blockVe[j].in.end()) {
									islive = true;
									break;
								}
							}
						}
						if (!islive) {
							if (debug) {
								cout << "%%%%%%%%%%%%%%" << i << " " << name << "%%%%%%%%%%%%%%%%%%%%\n";
							}
							queue<string> que;
							if (blockVe[i].midCodeVector[defPoi].x[0] == '#') {
								que.push(blockVe[i].midCodeVector[defPoi].x);
							}
							if (blockVe[i].midCodeVector[defPoi].y[0] == '#') {
								que.push(blockVe[i].midCodeVector[defPoi].y);
							}
							blockVe[i].midCodeVector.erase(blockVe[i].midCodeVector.begin() + defPoi, 
								blockVe[i].midCodeVector.begin() + defPoi + 1);  //defPoi删去
							while (!que.empty()) {
								string name = que.front();
								que.pop();
								for (int qi = defPoi; qi >= 0; qi--) {  //defPoi开始。。
									if (blockVe[i].midCodeVector[qi].z == name) {
										if (blockVe[i].midCodeVector[qi].x[0] == '#') {
											que.push(blockVe[i].midCodeVector[qi].x);
										}
										if (blockVe[i].midCodeVector[qi].y[0] == '#') {
											que.push(blockVe[i].midCodeVector[qi].y);
										}
										blockVe[i].midCodeVector.erase(blockVe[i].midCodeVector.begin() + qi, 
											blockVe[i].midCodeVector.begin() + qi + 1);  //qi删去
										break;
									}
								}
							}
							//新的blockVe更新到函数基本块表中
							funcBlockTable[funcName] = blockVe;
						}
					}
				}
			}
		}
	}
}

vector<string> unionVe(vector<string> v1, vector<string> v2) {
	vector<string> res;
	res.insert(res.end(), v1.begin(), v1.end());
	res.insert(res.end(), v2.begin(), v2.end());
	set<string> s(res.begin(), res.end());
	res.assign(s.begin(), s.end());
	return res;
}

vector<string> diffVe(vector<string> v1, vector<string> v2) {
	for (int i = 0; i < v2.size(); i++) {
		vector<string>::iterator iter = find(v1.begin(), v1.end(), v2[i]);
		if (iter != v1.end()) {
			v1.erase(iter);
		}
	}
	return v1;
}

void calInOut() {
	for (map<string, vector<Block> >::iterator it = funcBlockTable.begin(); it != funcBlockTable.end(); it++) {
		string funcName = (*it).first;
		vector<Block> blVe = (*it).second;
		while (true) {
			int cnt = 0;
			for (int i = blVe.size() - 1; i >= 0; i--) {
				vector<string> tOut, tIn, t;
				if (blVe[i].nextBlock1 != -1) {
					tOut = unionVe(tOut, blVe[blVe[i].nextBlock1].in);
				}
				if (blVe[i].nextBlock2 != -1) {
					tOut = unionVe(tOut, blVe[blVe[i].nextBlock2].in);
				}
				blVe[i].out = tOut;
				t = diffVe(tOut, blVe[i].def);
				tIn = unionVe(t, blVe[i].use);
				t = diffVe(tIn, blVe[i].in);
				if (t.size() != 0) {
					blVe[i].in = tIn;
				}
				else {
					cnt++;
				}
			}
			if (cnt == blVe.size()) {
				break;
			}
		}
		//当前是对blVe的修改 需要恢复到map中
		(*it).second = blVe;
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